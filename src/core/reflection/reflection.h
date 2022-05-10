#pragma once
#include <types/queue.h>
#include <types/string_hash.h>
#include <types/bitfield.h>
#include <types/array.h>
#include <types/string_view.h>
#include <types/string.h>
#include <filesystem/filesystem.h>
#include <glm/glm.hpp>
#include <yaml-cpp/yaml.h>
#include <core/resource_allocator/resource_allocator.h>

namespace YAML
{
	template <>
	struct convert<Vultr::String>
	{
		static Node encode(const Vultr::String &rhs)
		{
			Node node;
			node = rhs.c_str();
			return node;
		}

		static bool decode(const Node &node, Vultr::String &rhs)
		{
			if (!node)
				return false;
			rhs = Vultr::String(node.Scalar().c_str(), node.Scalar().size());
			return true;
		}
	};

	template <>
	struct convert<Vultr::Path>
	{
		static Node encode(const Vultr::Path &rhs)
		{
			Node node;
			node = rhs.c_str();
			return node;
		}

		static bool decode(const Node &node, Vultr::Path &rhs)
		{
			if (!node)
				return false;
			rhs = Vultr::Path(node.Scalar().c_str());
			return true;
		}
	};

	template <typename T>
	struct convert<Vultr::Resource<T>>
	{
		static Node encode(const Vultr::Resource<T> &rhs)
		{
			auto *allocator = resource_allocator<T>();
			Node node;
			node = allocator->get_resource_path(ResourceId(rhs).id).c_str();
			return node;
		}

		static bool decode(const Node &node, Vultr::Resource<T> &rhs)
		{
			if (node.IsNull())
			{
				rhs = {};
			}
			else
			{
				rhs = Vultr::Resource<T>(node.as<Vultr::Path>());
			}
			return true;
		}
	};

	template <>
	struct convert<Vec2>
	{
		static Node encode(const Vec2 &rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node &node, Vec2 &rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<f32>();
			rhs.y = node[1].as<f32>();
			return true;
		}
	};

	template <>
	struct convert<Vec3>
	{
		static Node encode(const Vec3 &rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node &node, Vec3 &rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<f32>();
			rhs.y = node[1].as<f32>();
			rhs.z = node[2].as<f32>();
			return true;
		}
	};

	template <>
	struct convert<Vec4>
	{
		static Node encode(const Vec4 &rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node &node, Vec4 &rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<f32>();
			rhs.y = node[1].as<f32>();
			rhs.z = node[2].as<f32>();
			rhs.w = node[3].as<f32>();
			return true;
		}
	};

	template <>
	struct convert<Quat>
	{
		static Node encode(const Quat &rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node &node, Quat &rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<f32>();
			rhs.y = node[1].as<f32>();
			rhs.z = node[2].as<f32>();
			rhs.w = node[3].as<f32>();
			return true;
		}
	};

	template <typename T, size_t len>
	struct convert<Vultr::Array<T, len>>
	{
		static Node encode(const Vultr::Array<T, len> &rhs)
		{
			Node node;
			for (auto &i : rhs)
				node.push_back(i);
			return node;
		}

		static bool decode(const Node &node, Vultr::Array<T, len> &rhs)
		{
			if (!node.IsSequence() || node.size() != len)
				return false;

			for (u32 i = 0; i < len; i++)
				rhs[i] = node[i].as<T>();
			return true;
		}
	};

	template <typename T>
	struct convert<Vultr::Vector<T>>
	{
		static Node encode(const Vultr::Vector<T> &rhs)
		{
			Node node;
			for (auto &i : rhs)
				node.push_back(i);
			return node;
		}

		static bool decode(const Node &node, Vultr::Vector<T> &rhs)
		{
			if (!node.IsSequence())
				return false;

			rhs.resize(node.size());
			for (u32 i = 0; i < rhs.size(); i++)
				rhs[i] = node[i].as<T>();

			return true;
		}
	};

	template <typename T>
	struct convert<Vultr::HashTable<T>>
	{
		static Node encode(const Vultr::HashTable<T> &rhs)
		{
			Node node;
			for (auto &i : rhs)
				node.push_back(i);
			return node;
		}

		static bool decode(const Node &node, Vultr::HashTable<T> &rhs)
		{
			if (!node.IsSequence())
				return false;

			for (auto &child : node)
			{
				if (rhs.contains(child.as<T>()))
					return false;
				rhs.set(child.as<T>());
			}

			return true;
		}
	};

	template <size_t len>
	struct convert<Vultr::Bitfield<len>>
	{
		static Node encode(const Vultr::Bitfield<len> &rhs)
		{
			Node node;

			char bits[len + 1];

			for (size_t i = 0; i < len; i++)
				bits[i] = rhs.at(i);

			node = bits;

			return node;
		}

		static bool decode(const Node &node, Vultr::Bitfield<len> &rhs)
		{
			if (!node)
				return false;

			const char *bits = node.Scalar().c_str();
			u32 bits_len     = node.Scalar().size();

			if (bits_len != len)
				return false;

			for (u32 i = 0; i < len; i++)
			{
				if (bits[i] != '0' && bits[i] != '1')
					return false;

				rhs.set(i, bits[i] == '1');
			}

			return true;
		}
	};

	template <>
	struct convert<Vultr::Buffer>
	{
		static Node encode(const Vultr::Buffer &rhs)
		{
			Node node;

			node = YAML::Binary(rhs.storage, rhs.size());

			return node;
		}

		static bool decode(const Node &node, Vultr::Buffer &rhs)
		{
			if (!node)
				return false;

			rhs.resize(node.size());
			rhs.fill(node.as<Binary>().data(), node.as<Binary>().size());

			return true;
		}
	};

} // namespace YAML

namespace Vultr
{
	inline YAML::Emitter &operator<<(YAML::Emitter &out, const String &s)
	{
		out << s.c_str();
		return out;
	}

	inline YAML::Emitter &operator<<(YAML::Emitter &out, const StringHash &s)
	{
		out << s.value();
		return out;
	}

	inline YAML::Emitter &operator<<(YAML::Emitter &out, const Path &p)
	{
		out << p.c_str();
		return out;
	}

	template <typename T>
	inline YAML::Emitter &operator<<(YAML::Emitter &out, const Resource<T> &r)
	{
		if (r.empty())
			return out << YAML::Null;
		auto *allocator = resource_allocator<T>();
		out << allocator->get_resource_path(ResourceId(r).id);
		return out;
	}

	inline YAML::Emitter &operator<<(YAML::Emitter &out, const Vec2 &v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter &operator<<(YAML::Emitter &out, const Vec3 &v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter &operator<<(YAML::Emitter &out, const Vec4 &v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter &operator<<(YAML::Emitter &out, const Quat &q)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << q.x << q.y << q.z << q.w << YAML::EndSeq;
		return out;
	}

	template <typename T, size_t len>
	inline YAML::Emitter &operator<<(YAML::Emitter &out, const Array<T, len> &a)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq;

		for (auto &i : a)
			out << i;

		out << YAML::EndSeq;
		return out;
	}

	template <typename T>
	inline YAML::Emitter &operator<<(YAML::Emitter &out, const Vector<T> &v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq;

		for (auto &i : v)
			out << i;

		out << YAML::EndSeq;
		return out;
	}

	template <typename T>
	inline YAML::Emitter &operator<<(YAML::Emitter &out, const HashTable<T> &v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq;

		for (auto &i : v)
			out << i;

		out << YAML::EndSeq;
		return out;
	}

	template <size_t len>
	inline YAML::Emitter &operator<<(YAML::Emitter &out, const Bitfield<len> &b)
	{
		char bits[len + 1];
		for (size_t i = 0; i < len; i++)
			bits[i] = b.at(i);

		out << bits;
		return out;
	}

	inline YAML::Emitter &operator<<(YAML::Emitter &out, const Buffer &b)
	{
		out << YAML::Binary(b.storage, b.size());
		return out;
	}

	enum struct PrimitiveType
	{
		U8,
		U16,
		U32,
		U64,
		S8,
		S16,
		S32,
		S64,
		F32,
		F64,
		CHAR,
		BYTE,
		BOOL,
		ARRAY,
		BITFIELD,
		BUFFER,
		VECTOR,
		HASHMAP,
		HASHTABLE,
		QUEUE,
		STRING,
		STRING_VIEW,
		STRING_HASH,
		PTR,
		VEC2,
		VEC3,
		VEC4,
		COLOR,
		QUAT,
		MAT3,
		MAT4,
		PATH,
		TEXTURE_RESOURCE,
		MESH_RESOURCE,
		SHADER_RESOURCE,
		MATERIAL_RESOURCE,
		NONE,
	};

	struct Field;

	struct Type
	{
		typedef void (*GetFieldsApi)(Field *out);
		typedef size_t (*GetSizeApi)();
		typedef void (*DeserializeApi)(const YAML::Node &node, const Field *field, void *data);
		typedef void (*CopyConstructorApi)(void *dest, const void *src);
		typedef YAML::Emitter &(*SerializeApi)(YAML::Emitter &, const Tuple<Field, void *> &);

		constexpr Type() = default;
		constexpr Type(PrimitiveType primitive_type, GetSizeApi get_size, SerializeApi serialize, DeserializeApi deserialize, CopyConstructorApi copy_constructor, StringView name, GetFieldsApi get_fields = nullptr,
					   u32 field_count = 0)
			: primitive_type(primitive_type), size(get_size), name(name), m_get_fields(get_fields), m_deserialize(deserialize), serialize(serialize), copy_constructor(copy_constructor), field_count(field_count),
			  id(name.hash())
		{
		}

		Vector<Field> get_fields() const;

		PrimitiveType primitive_type = PrimitiveType::NONE;
		StringView name{};
		u32 id                              = 0;
		GetFieldsApi m_get_fields           = nullptr;
		SerializeApi serialize              = nullptr;
		CopyConstructorApi copy_constructor = nullptr;
		GetSizeApi size                     = nullptr;
		DeserializeApi m_deserialize        = nullptr;
		u32 field_count                     = 0;
	};

	consteval Type init_type(StringView name, Type::GetSizeApi get_size, u32 field_count, Type::GetFieldsApi get_fields, Type::CopyConstructorApi copy_constructor)
	{
		return {PrimitiveType::NONE, get_size, nullptr, nullptr, copy_constructor, name, get_fields, field_count};
	}

	template <typename T>
	inline constexpr Type get_type;

	struct Field
	{
		typedef void *(*GetAddrApi)(void *);
		typedef void (*InitializeDefaultApi)(void *);
		constexpr Field() {}
		constexpr Field(StringView name, Type type, GetAddrApi get_addr, InitializeDefaultApi initialize_default)
			: name(name), type(type), m_get_addr(get_addr), m_initialize_default(initialize_default), id(name.hash())
		{
		}

		template <typename T>
		explicit consteval Field(StringView name, T *t) : name(name)
		{
			type = Type(t);
		}

		template <typename T>
		T *get_addr(void *data) const
		{
			return static_cast<T *>(m_get_addr(data));
		}

		ErrorOr<void> deserialize(const YAML::Node &node, void *data) const
		{
			if (type.m_deserialize == nullptr)
				return Error("No deserialization function found!");
			type.m_deserialize(node, this, data);
			return Success;
		}

		void initialize_default(void *data) const { m_initialize_default(get_addr<void *>(data)); }

		StringView name = "INVALID_FIELD";
		u32 id          = 0;
		Type type{};
		GetAddrApi m_get_addr                     = nullptr;
		InitializeDefaultApi m_initialize_default = nullptr;
	};

	consteval Field init_field(StringView name, Type type, Field::GetAddrApi get_addr, Field::InitializeDefaultApi initialize_default) { return {name, type, get_addr, initialize_default}; }

	inline Vector<Field> Type::get_fields() const
	{
		Field fields[field_count];
		m_get_fields(fields);
		return {fields, field_count};
	}

	template <typename T>
	YAML::Emitter &generic_type_serializer(YAML::Emitter &out, const Tuple<Field, void *> &field_data)
	{
		auto &[field, data] = field_data;
		out << *field.get_addr<T>(data);
		return out;
	}

	template <typename T>
	void generic_type_deserializer(const YAML::Node &node, const Field *field, void *data)
	{
		try
		{
			*field->get_addr<T>(data) = node.as<T>();
		}
		catch (const YAML::TypedBadConversion<T> &ex)
		{
			field->initialize_default(data);
		}
	}

	template <typename T>
	void generic_copy_constructor(void *dest, const void *src)
	{
		*static_cast<T *>(dest) = *static_cast<const T *>(src);
	}

	template <typename T>
	requires(is_pointer<T>) inline constexpr Type get_type<T> = {PrimitiveType::PTR, []() { return sizeof(T); }, generic_type_serializer<T>, "pointer"};

	template <typename T>
	requires(requires() { T::__REFL_TYPE; }) inline constexpr Type get_type<T> = T::__REFL_TYPE;

	template <>
	inline constexpr Type get_type<u8> = {PrimitiveType::U8, []() { return sizeof(u8); }, generic_type_serializer<u8>, generic_type_deserializer<u8>, generic_copy_constructor<u8>, "u8"};

	template <>
	inline constexpr Type get_type<u16> = {PrimitiveType::U16, []() { return sizeof(u16); }, generic_type_serializer<u16>, generic_type_deserializer<u16>, generic_copy_constructor<u16>, "u16"};

	template <>
	inline constexpr Type get_type<u32> = {PrimitiveType::U32, []() { return sizeof(u32); }, generic_type_serializer<u32>, generic_type_deserializer<u32>, generic_copy_constructor<u32>, "u32"};

	template <>
	inline constexpr Type get_type<u64> = {PrimitiveType::U64, []() { return sizeof(u64); }, generic_type_serializer<u64>, generic_type_deserializer<u64>, generic_copy_constructor<u64>, "u64"};

	template <>
	inline constexpr Type get_type<s8> = {PrimitiveType::S8, []() { return sizeof(s8); }, generic_type_serializer<s8>, generic_type_deserializer<s8>, generic_copy_constructor<s8>, "s8"};

	template <>
	inline constexpr Type get_type<s16> = {PrimitiveType::S16, []() { return sizeof(s16); }, generic_type_serializer<s16>, generic_type_deserializer<s16>, generic_copy_constructor<s16>, "s16"};

	template <>
	inline constexpr Type get_type<s32> = {PrimitiveType::S32, []() { return sizeof(s32); }, generic_type_serializer<s32>, generic_type_deserializer<s32>, generic_copy_constructor<s32>, "s32"};

	template <>
	inline constexpr Type get_type<s64> = {PrimitiveType::S64, []() { return sizeof(s64); }, generic_type_serializer<s64>, generic_type_deserializer<s64>, generic_copy_constructor<s64>, "s64"};

	template <>
	inline constexpr Type get_type<f32> = {PrimitiveType::F32, []() { return sizeof(f32); }, generic_type_serializer<f32>, generic_type_deserializer<f32>, generic_copy_constructor<f32>, "f32"};

	template <>
	inline constexpr Type get_type<f64> = {PrimitiveType::F64, []() { return sizeof(f64); }, generic_type_serializer<f64>, generic_type_deserializer<f64>, generic_copy_constructor<f64>, "f64"};

	template <>
	inline constexpr Type get_type<char> = {PrimitiveType::CHAR, []() { return sizeof(char); }, generic_type_serializer<char>, generic_type_deserializer<char>, generic_copy_constructor<char>, "char"};

	template <>
	inline constexpr Type get_type<bool> = {PrimitiveType::BOOL, []() { return sizeof(bool); }, generic_type_serializer<bool>, generic_type_deserializer<bool>, generic_copy_constructor<bool>, "byte"};

	template <typename T, size_t len>
	inline constexpr Type get_type<Array<T, len>> = {
		PrimitiveType::ARRAY, []() { return sizeof(Array<T, len>); }, generic_type_serializer<Array<T, len>>, generic_type_deserializer<Array<T, len>>, generic_copy_constructor<Array<T, len>>, "Array"};

	template <size_t len>
	inline constexpr Type get_type<Bitfield<len>> = {
		PrimitiveType::BITFIELD, []() { return sizeof(Bitfield<len>); }, generic_type_serializer<Bitfield>, generic_type_deserializer<Bitfield>, generic_copy_constructor<Bitfield>, "Bitfield"};

	template <>
	inline constexpr Type get_type<Buffer> = {PrimitiveType::BUFFER, []() { return sizeof(Buffer); }, generic_type_serializer<Buffer>, generic_type_deserializer<Buffer>, generic_copy_constructor<Buffer>, "Buffer"};

	template <typename T>
	inline constexpr Type get_type<Vector<T>> = {
		PrimitiveType::VECTOR, []() { return sizeof(Vector<T>); }, generic_type_serializer<Vector<T>>, generic_type_deserializer<Vector<T>>, generic_copy_constructor<Vector<T>>, "Vector"};

	template <typename K, typename V>
	inline constexpr Type get_type<Hashmap<K, V>> = {
		PrimitiveType::HASHMAP, []() { return sizeof(Hashmap<K, V>); }, generic_type_serializer<Hashmap<K, V>>, generic_type_deserializer<Hashmap<K, V>>, generic_copy_constructor<Hashmap<K, V>>, "Hashmap"};

	template <typename T>
	inline constexpr Type get_type<HashTable<T>> = {
		PrimitiveType::HASHTABLE, []() { return sizeof(HashTable<T>); }, generic_type_serializer<HashTable<T>>, generic_type_deserializer<HashTable<T>>, generic_copy_constructor<HashTable<T>>, "HashTable"};

	template <typename T>
	inline constexpr Type get_type<Queue<T>> = {
		PrimitiveType::QUEUE, []() { return sizeof(Queue<T>); }, generic_type_serializer<Queue<T>>, generic_type_deserializer<Queue<T>>, generic_copy_constructor<Queue<T>>, "Queue"};

	template <>
	inline constexpr Type get_type<StringView> = {PrimitiveType::STRING_VIEW, []() { return sizeof(StringView); }, generic_type_serializer<StringView>, nullptr, generic_copy_constructor<StringView>, "StringView"};

	template <>
	inline constexpr Type get_type<String> = {PrimitiveType::STRING, []() { return sizeof(String); }, generic_type_serializer<String>, generic_type_deserializer<String>, generic_copy_constructor<String>, "String"};

	template <>
	inline constexpr Type get_type<StringHash> = {PrimitiveType::STRING_HASH, []() { return sizeof(StringHash); }, generic_type_serializer<StringHash>, nullptr, generic_copy_constructor<StringHash>, "StringHash"};

	template <>
	inline constexpr Type get_type<Vec2> = {PrimitiveType::VEC2, []() { return sizeof(Vec2); }, generic_type_serializer<Vec2>, generic_type_deserializer<Vec2>, generic_copy_constructor<Vec2>, "Vec2"};

	template <>
	inline constexpr Type get_type<Vec3> = {PrimitiveType::VEC3, []() { return sizeof(Vec3); }, generic_type_serializer<Vec3>, generic_type_deserializer<Vec3>, generic_copy_constructor<Vec3>, "Vec3"};

	template <>
	inline constexpr Type get_type<Vec4> = {PrimitiveType::VEC4, []() { return sizeof(Vec4); }, generic_type_serializer<Vec4>, generic_type_deserializer<Vec4>, generic_copy_constructor<Vec4>, "Vec4"};

	template <>
	inline constexpr Type get_type<Quat> = {PrimitiveType::QUAT, []() { return sizeof(Quat); }, generic_type_serializer<Quat>, generic_type_deserializer<Quat>, generic_copy_constructor<Quat>, "Quat"};

	template <>
	inline constexpr Type get_type<Mat3> = {PrimitiveType::MAT3, []() { return sizeof(Mat3); }, generic_type_serializer<Mat3>, nullptr, generic_copy_constructor<Mat3>, "Mat3"};

	template <>
	inline constexpr Type get_type<Mat4> = {PrimitiveType::MAT4, []() { return sizeof(Mat4); }, generic_type_serializer<Mat4>, nullptr, generic_copy_constructor<Mat4>, "Mat4"};

	template <>
	inline constexpr Type get_type<Path> = {PrimitiveType::PATH, []() { return sizeof(Path); }, generic_type_serializer<Path>, generic_type_deserializer<Path>, generic_copy_constructor<Path>, "Path"};

} // namespace Vultr