// TODO: Reimplement using custom hashmap
#pragma once
#include <filesystem/virtual_filesystem.h>
#include <render/types/texture.h>
#include <render/types/shader.h>
#include <render/types/mesh.h>
#include <queue.h>
#include <dynamic_array.h>

namespace Vultr
{
    enum struct ResourceType : u8
    {
        INVALID = 0x0,
        TEXTURE = 0x1,
        SHADER = 0x2,
        MESH = 0x4,
        FONT = 0x8,
    };

    template <typename T>
    constexpr ResourceType get_resource_type();

    template <>
    inline constexpr ResourceType get_resource_type<Texture>()
    {
        return ResourceType::TEXTURE;
    }

    template <>
    inline constexpr ResourceType get_resource_type<Shader>()
    {
        return ResourceType::SHADER;
    }

    template <>
    inline constexpr ResourceType get_resource_type<Mesh>()
    {
        return ResourceType::MESH;
    }

    struct ResourceQueueItem
    {
        ResourceType type = ResourceType::INVALID;
        VFileHandle file = 0;
        void *temp_buf = nullptr;
    };

    // Load file data asynchronously on separate thread
    template <typename T>
    bool load_resource(const VirtualFilesystem *vfs, VFileHandle file, T *resource, ResourceQueueItem *item);

    template <typename T>
    bool finalize_resource(VFileHandle file, T *data, void *buffer);

    template <typename T>
    void free_resource(T *resource);

    template <typename T>
    struct ResourceData
    {
        bool loaded = false;
        u32 counter = 1;
        T data;
    };

    template <typename T>
    struct ResourceCache
    {
        vtl::DynamicArray<ResourceData<T>> cache;

        // std::unordered_map<size_t, VFileHandle> asset_to_index{};
        // std::unordered_map<VFileHandle, size_t> index_to_asset{};
    };

    struct ResourceManager
    {
        vtl::Queue<ResourceQueueItem> free_queue;
        vtl::Queue<ResourceQueueItem> load_queue;
        vtl::Queue<ResourceQueueItem> finalize_queue;

        ResourceCache<Texture> texture_cache;
        ResourceCache<Shader> shader_cache;
        ResourceCache<Mesh> mesh_cache;

        vtl::mutex mutex;

        ResourceManager() = default;
        ~ResourceManager() = default;

        template <typename T>
        void incr(VFileHandle file)
        {
            mutex.lock();

            auto *c = get_cache<T>();

            if (c->asset_to_index.find(file) != c->asset_to_index.end())
            {
                c->cache[c->asset_to_index[file]].counter++;
            }
            else
            {
                size_t new_index = c->cache.len;

                c->asset_to_index[file] = new_index;
                c->index_to_asset[new_index] = file;

                c->cache.push_back(ResourceData<T>());
                ResourceQueueItem item;
                item.file = file;
                item.type = get_resource_type<T>();
                load_queue.push(item);
            }

            mutex.unlock();
        }

        template <typename T>
        void decr(VFileHandle file)
        {
            mutex.lock();
            auto *c = get_cache<T>();

            assert(c->asset_to_index.find(file) != c->asset_to_index.end() && "Attempting to remove nonexistent asset");

            auto *count = &c->cache[c->asset_to_index[file]].counter;

            if (*count > 0)
            {
                (*count)--;
                ResourceQueueItem item;
                item.file = file;
                item.type = get_resource_type<T>();
                free_queue.push(item);
            }

            mutex.unlock();
        }

        template <typename T>
        T *get_asset(VFileHandle file)
        {
            auto *c = get_cache<T>();

            assert(has_asset<T>(file) && "Retreive nonexistent asset!");
            assert(is_asset_loaded<T>(file) && "Asset not loaded!");

            return &c->cache[c->asset_to_index.at(file)].data;
        }

        template <typename T>
        bool has_asset(VFileHandle file)
        {
            auto *c = get_cache<T>();
            return c->asset_to_index.find(file) != c->asset_to_index.end();
        }

        template <typename T>
        bool is_asset_loaded(VFileHandle file)
        {
            auto *c = get_cache<T>();

            assert(has_asset<T>(file) && "Nonexistent asset!");
            return c->cache[c->asset_to_index[file]].loaded;
        }

        template <typename T>
        ResourceCache<T> *get_cache();

        void load_asset(const VirtualFilesystem *vfs, ResourceQueueItem *item)
        {
            ResourceQueueItem res;
            switch (item->type)
            {
                case ResourceType::TEXTURE:
                    internal_load_asset<Texture>(vfs, item, &res);
                    break;
                case ResourceType::MESH:
                    internal_load_asset<Mesh>(vfs, item, &res);
                    break;
                case ResourceType::SHADER:
                    internal_load_asset<Shader>(vfs, item, &res);
                    break;
                case ResourceType::FONT:
                    assert(item->type != ResourceType::FONT && "Not implemented!");
                    break;
                case ResourceType::INVALID:
                default:
                    assert(item->type != ResourceType::INVALID && "Attempting to free invalid type!");
                    break;
            }

            if (res.type != ResourceType::INVALID)
            {
                finalize_queue.push(res);
            }
        }

        void finalize_assets(const VirtualFilesystem *vfs)
        {
            while (!finalize_queue.empty())
            {
                auto item = *finalize_queue.front();
                finalize_queue.pop();
                switch (item.type)
                {
                    case ResourceType::TEXTURE:
                        internal_finalize_asset<Texture>(vfs, &item);
                        break;
                    case ResourceType::MESH:
                        internal_finalize_asset<Mesh>(vfs, &item);
                        break;
                    case ResourceType::SHADER:
                        internal_finalize_asset<Shader>(vfs, &item);
                        break;
                    case ResourceType::FONT:
                        assert(item.type != ResourceType::FONT && "Not implemented!");
                        break;
                    case ResourceType::INVALID:
                    default:
                        assert(item.type != ResourceType::INVALID && "Attempting to free invalid type!");
                        break;
                }
            }
        }

        void garbage_collect()
        {
            // TODO: Dirty hack, this is not how this should be solved. This makes it so that if anything AT ALL is being queued to be loaded, then we can't remove any assets. The proper way of doing this would be
            // to allow garbage collection only when we are not actively loading any assets, however I'm not versed enough in threading to know how to solve this properly.
            if (load_queue.empty())
            {
                while (!free_queue.empty())
                {
                    auto item = *finalize_queue.front();
                    finalize_queue.pop();

                    switch (item.type)
                    {
                        case ResourceType::TEXTURE:
                            internal_garbage_collect<Texture>(&item);
                            break;
                        case ResourceType::MESH:
                            internal_garbage_collect<Mesh>(&item);
                            break;
                        case ResourceType::SHADER:
                            internal_garbage_collect<Shader>(&item);
                            break;
                        case ResourceType::FONT:
                            assert(item.type != ResourceType::FONT && "Not implemented!");
                            break;
                        case ResourceType::INVALID:
                        default:
                            assert(item.type != ResourceType::INVALID && "Attempting to free invalid type!");
                            break;
                    }
                }
            }
        }

        template <typename T>
        void internal_garbage_collect(ResourceQueueItem *item)
        {
            // auto *c = get_cache<T>();
            // assert(item->type == get_resource_type<T>() && "You've severely fucked up if you got to this point.");
            // auto asset = item->file;

            // auto index_of_removed_asset = c->asset_to_index[asset];
            // auto index_of_last_element = c->cache.len - 1;

            // // Free the resource first
            // T *resource = &c->cache[index_of_removed_asset].data;
            // free_resource<T>(resource);

            // // Then replace the removed data with data from the last element
            // c->cache[index_of_removed_asset] = c->cache[index_of_last_element];

            // // Update the maps for the newly moved element
            // VFileHandle asset_of_last_element = c->index_to_asset[index_of_last_element];
            // c->asset_to_index[asset_of_last_element] = index_of_removed_asset;
            // c->index_to_asset[index_of_removed_asset] = asset_of_last_element;

            // // Remove the asset requested from the maps
            // c->asset_to_index.erase(asset);
            // c->index_to_asset.erase(index_of_last_element);

            // // Remove the data from the last asset from our array
            // c->cache.remove_last();
        }

        template <typename T>
        void internal_load_asset(const VirtualFilesystem *vfs, ResourceQueueItem *item, ResourceQueueItem *res)
        {
            auto *c = get_cache<T>();
            // load_resource<T>(vfs, item->file, &c->cache[c->asset_to_index[item->file]].data, res);
        }

        template <typename T>
        void internal_finalize_asset(const VirtualFilesystem *vfs, ResourceQueueItem *item)
        {
            auto *c = get_cache<T>();
            // finalize_resource<T>(item->file, &c->cache[c->asset_to_index[item->file]].data, item->temp_buf);
        }

        template <>
        ResourceCache<Texture> *get_cache()
        {
            return &texture_cache;
        }

        template <>
        ResourceCache<Shader> *get_cache()
        {
            return &shader_cache;
        }

        template <>
        ResourceCache<Mesh> *get_cache()
        {
            return &mesh_cache;
        }
    };
} // namespace Vultr
