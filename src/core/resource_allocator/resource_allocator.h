#pragma once
#include <types/types.h>
#include <types/hashmap.h>
#include <types/queue.h>
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
		ERROR        = 0x4,
	};

	template <typename T>
	requires(is_pointer<T>) struct ResourceInfo
	{
		u32 count = 1;
		Path path{};
		ResourceLoadState load_state = ResourceLoadState::NEED_TO_LOAD;
		T data                       = nullptr;
		Option<Error> error          = None;
	};

	struct ResourceLoadItem
	{
		u32 id = 0;
		Path path{};
		bool is_kill_request = false;
	};

	template <typename T>
	requires(is_pointer<T>) struct ResourceAllocator
	{
		explicit ResourceAllocator() = default;
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
				if (free_queue_listener != nullptr)
				{
					free_queue_listener->push(info.data);
				}
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
			Platform::Lock lock(mutex);
			ASSERT(resources.contains(id), "Cannot get invalid resources %u", id);
			return resources.get(id).load_state == ResourceLoadState::LOADED;
		}

		bool has_error(u32 id)
		{
			Platform::Lock lock(mutex);
			ASSERT(resources.contains(id), "Cannot get invalid resource %u", id);
			return resources.get(id).load_state == ResourceLoadState::ERROR;
		}

		Error get_error(u32 id)
		{
			Platform::Lock lock(mutex);
			ASSERT(has_error(id), "Cannot get error from resource which does not have an error!");
			return resources.get(id).error.value();
		}

		ResourceLoadItem wait_pop_load_queue()
		{
			while (true)
			{
				u32 res = load_queue.pop_wait();
				if (res == U32Max)
					return {.is_kill_request = true};

				Platform::Lock lock(mutex);

				if (!resources.contains(res) || resources.get(res).load_state != ResourceLoadState::NEED_TO_LOAD)
					continue;

				resources.get(res).load_state = ResourceLoadState::LOADING;
				return {.id = res, .path = resources.get(res).path, .is_kill_request = false};
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

		ErrorOr<void> add_loaded_resource_error(u32 id, const Error &error)
		{
			Platform::Lock lock(mutex);
			if (!resources.contains(id))
				return Error("Resource has already been freed before it was loaded!");

			auto &resource = resources.get(id);
			ASSERT(resource.load_state != ResourceLoadState::LOADED, "Resource was loaded before an error was set");

			resource.error      = error;
			resource.load_state = ResourceLoadState::ERROR;
			return Success;
		}

		void kill_loading_threads()
		{
			load_queue.clear();
			load_queue.push(U32Max);
			while (!load_queue.empty())
			{
			}
		}

		void kill_freeing_threads()
		{
			for (auto &[id, info] : resources)
			{
				info.count = 1;
				decr(id);
			}
			free_queue.push((T)-1);
			while (!free_queue.empty())
			{
			}
		}

		Path get_resource_path(u32 id)
		{
			Platform::Lock lock(mutex);
			return resources.get(id).path;
		}

		void notify_reload(const Path &path)
		{
			Platform::Lock lock(mutex);
			for (auto &[id, info] : resources)
			{
				if (info.path == path)
				{
					if (info.load_state == ResourceLoadState::LOADED)
					{
						free_queue.push(info.data);
						if (free_queue_listener != nullptr)
						{
							free_queue_listener->push(info.data);
						}
					}
					info.error      = None;
					info.data       = nullptr;
					info.load_state = ResourceLoadState::NEED_TO_LOAD;
					load_queue.push(id);
					return;
				}
			}
		}

		Hashmap<u32, ResourceInfo<T>> resources{};
		Queue<u32, 1024> load_queue{};
		Queue<T, 1024> free_queue{};
		Queue<void *, 1024> *free_queue_listener = nullptr;
		Platform::Mutex mutex{};
	};

	template <typename T>
	ResourceAllocator<T> *resource_allocator();

	template <typename T>
	requires(is_pointer<T>) struct Resource;

	struct ResourceId
	{
		explicit consteval ResourceId(StringHash hash) : id(hash.value()) {}
		explicit constexpr ResourceId(u32 id) : id(id) {}
		constexpr ResourceId(const ResourceId &other) : id(other.id) {}
		ResourceId &operator=(const ResourceId &other)
		{
			id = other.id;
			return *this;
		}

		template <typename T>
		ResourceId(const Resource<T> &other);
		template <typename T>
		ResourceId &operator=(const Resource<T> &other);

		constexpr bool operator==(const ResourceId &other) const { return id == other.id; }
		constexpr operator u32() const { return id; }

		template <typename T>
		bool loaded() const
		{
			return id && resource_allocator<T>()->is_loaded(id);
		}

		template <typename T>
		ErrorOr<T> try_value() const
		{
			return resource_allocator<T>()->try_get_loaded_resource(id);
		}

		template <typename T>
		T value() const
		{
			CHECK_UNWRAP(T res, resource_allocator<T>()->try_get_loaded_resource(id));
			return res;
		}

		template <typename T>
		T value_or(T other) const
		{
			if check (try_value<T>(), auto value, auto _)
			{
				return value;
			}
			else
			{
				return other;
			}
		}

		u32 id;
	};

	template <>
	struct Traits<ResourceId> : GenericTraits<ResourceId>
	{
		static constexpr u32 hash(const ResourceId value) { return value.id; }
		static constexpr bool equals(const ResourceId a, const ResourceId b) { return a == b; }
		static constexpr bool is_trivial() { return false; }
	};

	template <typename T>
	requires(is_pointer<T>) struct Resource
	{
		Resource() = default;
		explicit Resource(StringHash hash) : id(hash.value()) { resource_allocator<T>()->incr(id.value(), Path(hash.c_str())); }

		explicit Resource(const Path &path) : id(Traits<StringView>::hash(path.string())) { resource_allocator<T>()->incr(id.value(), path); }
		explicit Resource(const ResourceId &other) : id(other.id)
		{
			if (id.has_value())
				resource_allocator<T>()->incr(id.value());
		}
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

		bool operator==(const Resource &other) const { return this->id == other.id; }
		bool operator==(const ResourceId &other) const { return this->id == other.id; }

		bool loaded() const { return id.has_value() && resource_allocator<T>()->is_loaded(id.value()); }
		bool has_error() const { return id.has_value() && resource_allocator<T>()->has_error(id.value()); }
		Error get_error() const { return resource_allocator<T>()->get_error(id.value()); }
		void wait_loaded() const
		{
			while (!loaded())
			{
			}
		}

		ErrorOr<T> try_value() const
		{
			if (!id.has_value())
				return Error("Invalid empty resource!");

			return resource_allocator<T>()->try_get_loaded_resource(id.value());
		}

		T value() const
		{
			CHECK_UNWRAP(T res, resource_allocator<T>()->try_get_loaded_resource(id.value()));
			return res;
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

		bool empty() const { return !id.has_value(); }

		constexpr operator ResourceId() { return ResourceId(id.value_or(0)); }

	  private:
		Option<u32> id = None;

		friend ResourceId;
		friend Traits<Resource<T>>;
	};

	template <typename T>
	ResourceId::ResourceId(const Resource<T> &other) : id(other.id.value())
	{
	}

	template <typename T>
	ResourceId &ResourceId::operator=(const Resource<T> &other)
	{
		id = other.id.value();
		return *this;
	}

	template <typename T>
	struct Traits<Resource<T>> : GenericTraits<Resource<T>>
	{
		static constexpr u32 hash(const Resource<T> &value) { return value.id.value_or(0); }
		static constexpr bool equals(const Resource<T> &a, const Resource<T> &b) { return a == b; }
		static constexpr bool is_trivial() { return false; }
	};

} // namespace Vultr