#pragma once
#include <types/types.h>
#include <types/bitfield.h>
#include <types/tuple.h>
#include <core/reflection/reflection.h>

namespace Vultr
{
	static constexpr size_t MAX_COMPONENTS = 128;
	typedef Bitfield<MAX_COMPONENTS> Signature;

#define VCOMPONENT_BEGIN(name)                                                                                                                                                                                        \
	struct name                                                                                                                                                                                                       \
	{                                                                                                                                                                                                                 \
		static constexpr Vultr::StringView __REFL_NAME = #name;                                                                                                                                                       \
		static constexpr u32 __REFL_START_COUNT        = __COUNTER__ + 1;                                                                                                                                             \
		template <size_t i>                                                                                                                                                                                           \
		static constexpr Vultr::Field __REFL_GET_FIELD;                                                                                                                                                               \
		static constexpr name *__REFL_CAST(void *ptr) { return static_cast<name *>(ptr); }                                                                                                                            \
		static constexpr size_t __REFL_GET_SIZE() { return sizeof(name); }                                                                                                                                            \
		static constexpr void __REFL_COPY_CONSTRUCTOR(void *dest, const void *src) { *static_cast<name *>(dest) = *static_cast<const name *>(src); }

#define VCOMPONENT_FIELD(T, name, val)                                                                                                                                                                                \
	T name = val;                                                                                                                                                                                                     \
	static constexpr void *__REFL_##name##_GET_ADDR(void *component) { return &__REFL_CAST(component)->name; }                                                                                                        \
	template <>                                                                                                                                                                                                       \
	static constexpr Vultr::Field __REFL_GET_FIELD<__COUNTER__ - __REFL_START_COUNT> = Vultr::init_field(#name, Vultr::get_type<T>, __REFL_##name##_GET_ADDR, [](void *data) {                                        \
		new (data) T();                                                                                                                                                                                               \
		*static_cast<T *>(data) = val;                                                                                                                                                                                \
	});

#define VCOMPONENT_END()                                                                                                                                                                                              \
	static constexpr u32 __REFL_MEMBER_COUNT = __COUNTER__ - __REFL_START_COUNT;                                                                                                                                      \
	template <size_t... S>                                                                                                                                                                                            \
	static constexpr void __REFL_GET_FIELDS_IMPL(Vultr::Field *out, Vultr::Sequence<S...>)                                                                                                                            \
	{                                                                                                                                                                                                                 \
		((out[S] = __REFL_GET_FIELD<S>), ...);                                                                                                                                                                        \
	}                                                                                                                                                                                                                 \
	static constexpr void __REFL_GET_FIELDS(Vultr::Field *out) { __REFL_GET_FIELDS_IMPL(out, typename Vultr::SequenceImpl<__REFL_MEMBER_COUNT>::type()); }                                                            \
	static constexpr Vultr::Type __REFL_TYPE = Vultr::init_type(__REFL_NAME, __REFL_GET_SIZE, __REFL_MEMBER_COUNT, __REFL_GET_FIELDS, __REFL_COPY_CONSTRUCTOR);                                                       \
	}                                                                                                                                                                                                                 \
	;

	struct EditorField
	{
		EditorField() = default;
		EditorField(const Field &field, void *addr) : field(field), addr(addr) {}

		template <typename T>
		T *get_addr() const
		{
			return static_cast<T *>(addr);
		}

		Field field{};
		void *addr = nullptr;
	};

	struct EditorType
	{
		EditorType() = default;
		EditorType(const Type &type, void *data) : type(type), data(data)
		{
			for (auto &field : type.get_fields())
			{
				fields.push_back({field, field.get_addr<void>(data)});
			}
		}
		EditorType(const Type &type, void *data, const Vector<EditorField> &fields) : type(type), data(data), fields(fields) {}

		Type type{};
		Vector<EditorField> fields{};
		void *data = nullptr;
	};

	template <typename T>
	inline EditorType get_editor_type(T *component)
	{
		return EditorType(get_type<T>, component);
	}

} // namespace Vultr