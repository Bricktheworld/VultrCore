#pragma once
#include <types/types.h>
#include <types/bitfield.h>

namespace Vultr
{
	static constexpr size_t MAX_COMPONENTS = 128;
	typedef Bitfield<MAX_COMPONENTS> Signature;

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
		VOID_PTR,
		VEC2,
		VEC3,
		VEC4,
		COLOR,
		QUAT,
		PATH,
		OPTIONAL_PATH,
		OTHER,
	};

	struct ComponentMember
	{
		StringView name    = nullptr;
		PrimitiveType type = PrimitiveType::OTHER;
		void *addr         = nullptr;
	};

	template <typename T>
	struct ComponentTraits : public ReflTraits<T>
	{
		static Vector<ComponentMember> members(T *component);
		static consteval u32 component_id() { return ReflTraits<T>::type_id(); }
	};

	//#define COMPONENT_TRAITS(Type)                                                                                                                                                                                        \
//	template <>                                                                                                                                                                                                       \
//	struct ReflTraits<Type> : public Traits<Type>                                                                                                                                                                     \
//	{                                                                                                                                                                                                                 \
//		static consteval StringView type_name() { return #Type; }                                                                                                                                                     \
//	}
	//
} // namespace Vultr