#pragma once
#include <types/string_view.h>
#include <types/string.h>
#include <glm/glm.hpp>

namespace Vultr
{
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
		STRING_VIEW,
		STRING,
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

		constexpr Type() = default;
		constexpr Type(PrimitiveType primitive_type, StringView name, GetFieldsApi get_fields = nullptr, u32 field_count = 0)
			: primitive_type(primitive_type), name(name), m_get_fields(get_fields), field_count(field_count), id(name.hash())
		{
		}

		Vector<Field> get_fields() const;

		PrimitiveType primitive_type = PrimitiveType::NONE;
		StringView name{};
		u32 id                    = 0;
		GetFieldsApi m_get_fields = nullptr;
		u32 field_count           = 0;
	};

	consteval Type init_type(StringView name, u32 field_count, Type::GetFieldsApi get_fields) { return {PrimitiveType::NONE, name, get_fields, field_count}; }

	template <typename T>
	consteval Type init_type()
	{
		if constexpr (is_pointer<T>)
		{
			return {PrimitiveType::PTR, "pointer"};
		}

		static_assert(
			requires() { T::__REFL_TYPE; }, "Cannot get reflection type because __REFL_TYPE does not exist!");

		return {
			.primitive_type = T::__REFL_TYPE.primitive_type,
			.name           = T::__REFL_TYPE.name,
			.get_fields     = T::__REFL_TYPE.get_fields,
			.field_count    = T::__REFL_TYPE.field_count,
		};
	}

	template <>
	consteval Type init_type<u8>()
	{
		return {PrimitiveType::U8, "u8"};
	}

	template <>
	consteval Type init_type<u16>()
	{
		return {PrimitiveType::U16, "u16"};
	}

	template <>
	consteval Type init_type<u32>()
	{
		return {PrimitiveType::U32, "u32"};
	}

	template <>
	consteval Type init_type<u64>()
	{
		return {PrimitiveType::U64, "u64"};
	}

	template <>
	consteval Type init_type<s8>()
	{
		return {PrimitiveType::S8, "s8"};
	}

	template <>
	consteval Type init_type<s16>()
	{
		return {PrimitiveType::S16, "s16"};
	}

	template <>
	consteval Type init_type<s32>()
	{
		return {PrimitiveType::S32, "s32"};
	}

	template <>
	consteval Type init_type<s64>()
	{
		return {PrimitiveType::S64, "s64"};
	}

	template <>
	consteval Type init_type<f32>()
	{
		return {PrimitiveType::F32, "f32"};
	}

	template <>
	consteval Type init_type<f64>()
	{
		return {PrimitiveType::F64, "f64"};
	}

	template <>
	consteval Type init_type<char>()
	{
		return {PrimitiveType::CHAR, "char"};
	}

	template <>
	consteval Type init_type<bool>()
	{
		return {PrimitiveType::BOOL, "byte"};
	}

	template <>
	consteval Type init_type<StringView>()
	{
		return {PrimitiveType::STRING_VIEW, "StringView"};
	}

	template <>
	consteval Type init_type<String>()
	{
		return {PrimitiveType::STRING, "String"};
	}

	template <>
	consteval Type init_type<Vec2>()
	{
		return {PrimitiveType::VEC2, "Vec2"};
	}

	template <>
	consteval Type init_type<Vec3>()
	{
		return {PrimitiveType::VEC3, "Vec3"};
	}

	template <>
	consteval Type init_type<Vec4>()
	{
		return {PrimitiveType::VEC4, "Vec4"};
	}

	template <>
	consteval Type init_type<Quat>()
	{
		return {PrimitiveType::QUAT, "Quat"};
	}

	template <>
	consteval Type init_type<Mat3>()
	{
		return {PrimitiveType::MAT3, "Mat3"};
	}

	template <>
	consteval Type init_type<Mat4>()
	{
		return {PrimitiveType::MAT4, "Mat4"};
	}

	template <>
	consteval Type init_type<Path>()
	{
		return {PrimitiveType::PATH, "Path"};
	}

	struct Field
	{
		typedef void *(*GetAddrApi)(void *);
		constexpr Field() {}
		constexpr Field(StringView name, Type type, GetAddrApi get_addr) : name(name), type(type), m_get_addr(get_addr) {}

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

		StringView name = "INVALID_FIELD";
		Type type{};
		GetAddrApi m_get_addr = nullptr;
	};

	consteval Field init_field(StringView name, Type type, Field::GetAddrApi get_addr) { return {name, type, get_addr}; }

	inline Vector<Field> Type::get_fields() const
	{
		Field fields[field_count];
		m_get_fields(fields);
		return {fields, field_count};
	}

	namespace Refl
	{
		template <typename T>
		requires(requires() { T::__REFL_TYPE; }) constexpr Type get_type() { return T::__REFL_TYPE; }

	} // namespace Refl
} // namespace Vultr