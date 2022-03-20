#pragma once
#include <types/types.h>
#include <types/hashmap.h>
#include <types/static_details.h>
#include <types/string_view.h>
#include <types/string_hash.h>
#include <filesystem/filesystem.h>

namespace Vultr
{
	enum struct ResourceLoadState : u8
	{
		NEED_TO_LOAD = 0x0,
		LOADING      = 0x1,
		LOADED       = 0x2,
	};

	template <typename T>
	requires(is_pointer<T>) struct ResourceInfo
	{
		u32 count = 1;
		Path path{};
		ResourceLoadState load_state = ResourceLoadState::NEED_TO_LOAD;
		T data                       = nullptr;
	};

	struct ResourceLoadItem
	{
		u32 id = 0;
		Path path{};
	};

	template <typename T>
	requires(is_pointer<T>) struct ResourceAllocator
	{
		explicit ResourceAllocator(const Path &resource_dir) : resource_dir(resource_dir) {}
		void incr(u32 id, const Path &path)
		{
			Platform::Lock lock(mutex);

			if (!resources.contains(id))
			{
				resources.set(id, {.count = 1, .path = path});
				load_queue.push(id);
			}
			else
			{
				resources.get(id).count++;
			}
		}

		void incr(u32 id)
		{
			Platform::Lock lock(mutex);
			ASSERT(resources.contains(id), "Cannot increment non-existent resource!");
			resources.get(id).count++;
		}

		void decr(u32 id)
		{
			Platform::Lock lock(mutex);
			ASSERT(resources.contains(id), "Cannot decrement non-existent resource %u", id);

			auto count = --resources.get(id).count;
			if (count > 0)
				return;

			auto info = resources.get(id);
			resources.remove(id);
			if (info.load_state == ResourceLoadState::LOADED)
			{
				free_queue.push(info.data);
			}
		}

		ErrorOr<T> try_get_loaded_resource(u32 id)
		{
			Platform::Lock lock(mutex);
			ASSERT(resources.contains(id), "Cannot get invalid resources %u", id);
			if (resources.get(id).load_state == ResourceLoadState::LOADED)
			{
				return resources.get(id).data;
			}
			else
			{
				return Error("Resource not yet loaded!");
			}
		}

		bool is_loaded(u32 id)
		{
			ASSERT(resources.contains(id), "Cannot get invalid resources %u", id);
			return resources.get(id).load_state == ResourceLoadState::LOADED;
		}

		ResourceLoadItem wait_pop_load_queue()
		{
			while (true)
			{
				u32 res = load_queue.pop_wait();

				Platform::Lock lock(mutex);

				if (!resources.contains(res) || resources.get(res).load_state != ResourceLoadState::NEED_TO_LOAD)
					continue;

				resources.get(res).load_state = ResourceLoadState::LOADING;
				return {.id = res, .path = resources.get(res).path};
			}
		}

		T wait_pop_free_queue() { return free_queue.pop_wait(); }

		ErrorOr<void> add_loaded_resource(u32 id, T data)
		{
			ASSERT(data != nullptr, "Cannot load with nullptr data!");
			Platform::Lock lock(mutex);
			if (!resources.contains(id))
				return Error("Resource has already been freed before it was loaded!");

			auto &resource = resources.get(id);

			ASSERT(resource.load_state != ResourceLoadState::LOADED, "Resource was loaded twice!");

			resource.data       = data;
			resource.load_state = ResourceLoadState::LOADED;
			return Success;
		}

		Path resource_dir{};
		Hashmap<u32, ResourceInfo<T>> resources{};
		Queue<u32, 1024> load_queue{};
		Queue<T, 1024> free_queue{};
		Platform::Mutex mutex{};
	};

	template <typename T>
	ResourceAllocator<T> *resource_allocator();

	template <typename T>
	requires(is_pointer<T>) struct Resource
	{
		explicit Resource(StringHash hash) : id(hash.value()) { resource_allocator<T>()->incr(id.value(), Path(hash.c_str())); }

		explicit Resource(const Path &path) : id(Traits<StringView>::hash(path.string())) { resource_allocator<T>()->incr(id.value(), path); }
		~Resource()
		{
			if (!id.has_value())
				return;

			resource_allocator<T>()->decr(id.value());
			id = None;
		}

		Resource(const Resource &other)
		{
			this->~Resource<T>();

			if (!other.id.has_value())
				return;

			id = other.id;
			resource_allocator<T>()->incr(id.value());
		}

		Resource(Resource &&other)
		{
			this->~Resource<T>();

			if (!other.id.has_value())
				return;

			id       = other.id;
			other.id = None;
		}

		Resource &operator=(const Resource &other)
		{
			this->~Resource<T>();

			if (!other.id.has_value())
				return *this;

			id = other.id;
			resource_allocator<T>()->incr(id.value());
			return *this;
		}
		Resource &operator=(Resource &&other)
		{
			this->~Resource<T>();

			if (!other.id.has_value())
				return *this;

			id       = other.id;
			other.id = None;
			return *this;
		}

		bool loaded() const
		{
			ASSERT(id.has_value(), "Cannot get loaded from invalid resource!");

			return resource_allocator<T>()->is_loaded(id.value());
		}

		ErrorOr<T> try_value() const
		{
			if (!id.has_value())
				return Error("Invalid empty resource!");

			return resource_allocator<T>()->try_get_loaded_resource(id.value());
		}

		T value_or(T other) const
		{
			if check (try_value(), auto value, auto _)
			{
				return value;
			}
			else
			{
				return other;
			}
		}

	  private:
		Option<u32> id = None;
	};

} // namespace Vultr