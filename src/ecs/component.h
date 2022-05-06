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
		static constexpr StringView __REFL_NAME = #name;                                                                                                                                                              \
		static constexpr u32 __REFL_START_COUNT = __COUNTER__ + 1;                                                                                                                                                    \
		template <size_t i>                                                                                                                                                                                           \
		static constexpr Field __REFL_GET_FIELD;                                                                                                                                                                      \
		static constexpr name *__REFL_CAST(void *ptr) { return static_cast<name *>(ptr); }                                                                                                                            \
		static constexpr size_t __REFL_GET_SIZE() { return sizeof(name); }

#define VCOMPONENT_FIELD(T, name, val)                                                                                                                                                                                \
	T name = val;                                                                                                                                                                                                     \
	static constexpr void *__REFL_##name##_GET_ADDR(void *component) { return &__REFL_CAST(component)->name; }                                                                                                        \
	template <>                                                                                                                                                                                                       \
	static constexpr Field __REFL_GET_FIELD<__COUNTER__ - __REFL_START_COUNT> = init_field(#name, get_type<T>, __REFL_##name##_GET_ADDR, [](void *data) {                                                             \
		new (data) T();                                                                                                                                                                                               \
		*static_cast<T *>(data) = val;                                                                                                                                                                                \
	});

#define VCOMPONENT_END()                                                                                                                                                                                              \
	static constexpr u32 __REFL_MEMBER_COUNT = __COUNTER__ - __REFL_START_COUNT;                                                                                                                                      \
	template <size_t... S>                                                                                                                                                                                            \
	static constexpr void __REFL_GET_FIELDS_IMPL(Field *out, Sequence<S...>)                                                                                                                                          \
	{                                                                                                                                                                                                                 \
		((out[S] = __REFL_GET_FIELD<S>), ...);                                                                                                                                                                        \
	}                                                                                                                                                                                                                 \
	static constexpr void __REFL_GET_FIELDS(Field *out) { __REFL_GET_FIELDS_IMPL(out, typename SequenceImpl<__REFL_MEMBER_COUNT>::type()); }                                                                          \
	static constexpr Type __REFL_TYPE = init_type(__REFL_NAME, __REFL_GET_SIZE, __REFL_MEMBER_COUNT, __REFL_GET_FIELDS);                                                                                              \
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