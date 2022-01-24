#pragma once

namespace Vultr
{
	template <typename T>
	inline constexpr bool is_trivial = __is_trivial(T);

	template <class T>
	inline constexpr bool is_l_value = false;

	template <class T>
	inline constexpr bool is_l_value<T &> = true;

	template <class T>
	inline constexpr bool is_r_value = false;
	template <class T>
	inline constexpr bool is_r_value<T &&> = true;

	template <typename T, typename U>
	inline constexpr bool is_same = false;

	template <typename T>
	inline constexpr bool is_same<T, T> = true;

	template <typename Base, typename Derived>
	inline constexpr bool is_base_of = __is_base_of(Base, Derived);

	template <class T>
	using add_const = const T;

	template <class T>
	struct __remove_const
	{
		using Type = T;
	};
	template <class T>
	struct __remove_const<const T>
	{
		using Type = T;
	};
	template <class T>
	using remove_const = typename __remove_const<T>::Type;

	template <class T>
	struct __remove_volatile
	{
		using Type = T;
	};

	template <class T>
	struct __remove_volatile<volatile T>
	{
		using Type = T;
	};

	template <typename T>
	using remove_volatile = typename __remove_volatile<T>::Type;

	template <class T>
	using remove_cv = remove_volatile<remove_const<T>>;

	template <class T>
	inline constexpr bool __IsPointerHelper = false;

	template <class T>
	inline constexpr bool __IsPointerHelper<T *> = true;

	template <class T>
	inline constexpr bool is_pointer = __IsPointerHelper<remove_cv<T>>;

	template <typename... Ts>
	struct IndexOf;

	template <typename T, typename... R>
	struct IndexOf<T, T, R...>
	{
		static constexpr size_t index = 0;
	};

	template <typename T, typename F, typename... R>
	struct IndexOf<T, F, R...>
	{
		static constexpr size_t index = 1 + IndexOf<T, R...>::index;
	};

	template <typename... Ts>
	struct Contains;

	template <typename T, typename... R>
	struct Contains<T, T, R...>
	{
		static constexpr bool contains = true;
	};

	template <typename T, typename F, typename... R>
	struct Contains<T, F, R...>
	{
		static constexpr bool contains = Contains<T, R...>::contains;
	};

	template <typename T, typename F>
	struct Contains<T, F>
	{
		static constexpr bool contains = false;
	};

	template <typename T>
	struct __MakeUnsigned
	{
		using Type = void;
	};
	template <>
	struct __MakeUnsigned<signed char>
	{
		using Type = unsigned char;
	};
	template <>
	struct __MakeUnsigned<short>
	{
		using Type = unsigned short;
	};
	template <>
	struct __MakeUnsigned<int>
	{
		using Type = unsigned int;
	};
	template <>
	struct __MakeUnsigned<long>
	{
		using Type = unsigned long;
	};
	template <>
	struct __MakeUnsigned<long long>
	{
		using Type = unsigned long long;
	};
	template <>
	struct __MakeUnsigned<unsigned char>
	{
		using Type = unsigned char;
	};
	template <>
	struct __MakeUnsigned<unsigned short>
	{
		using Type = unsigned short;
	};
	template <>
	struct __MakeUnsigned<unsigned int>
	{
		using Type = unsigned int;
	};
	template <>
	struct __MakeUnsigned<unsigned long>
	{
		using Type = unsigned long;
	};
	template <>
	struct __MakeUnsigned<unsigned long long>
	{
		using Type = unsigned long long;
	};
	template <>
	struct __MakeUnsigned<char>
	{
		using Type = unsigned char;
	};
	template <>
	struct __MakeUnsigned<char8_t>
	{
		using Type = char8_t;
	};
	template <>
	struct __MakeUnsigned<char16_t>
	{
		using Type = char16_t;
	};
	template <>
	struct __MakeUnsigned<char32_t>
	{
		using Type = char32_t;
	};
	template <>
	struct __MakeUnsigned<bool>
	{
		using Type = bool;
	};

	template <typename T>
	using make_unsigned = typename __MakeUnsigned<T>::Type;

	template <typename T>
	inline constexpr bool __IsIntegral = false;

	template <>
	inline constexpr bool __IsIntegral<bool> = true;
	template <>
	inline constexpr bool __IsIntegral<unsigned char> = true;
	template <>
	inline constexpr bool __IsIntegral<char8_t> = true;
	template <>
	inline constexpr bool __IsIntegral<char16_t> = true;
	template <>
	inline constexpr bool __IsIntegral<char32_t> = true;
	template <>
	inline constexpr bool __IsIntegral<unsigned short> = true;
	template <>
	inline constexpr bool __IsIntegral<unsigned int> = true;
	template <>
	inline constexpr bool __IsIntegral<unsigned long> = true;
	template <>
	inline constexpr bool __IsIntegral<unsigned long long> = true;

	template <typename T>
	inline constexpr bool is_integral = __IsIntegral<make_unsigned<remove_cv<T>>>;

	template <typename T>
	struct __RemoveReference
	{
		using Type = T;
	};
	template <class T>
	struct __RemoveReference<T &>
	{
		using Type = T;
	};
	template <class T>
	struct __RemoveReference<T &&>
	{
		using Type = T;
	};

	template <typename T>
	using remove_reference = typename __RemoveReference<T>::Type;

	template <typename T>
	using remove_cv_reference = remove_cv<remove_reference<T>>;

	template <class T>
	struct __RemovePointer
	{
		using Type = T;
	};
	template <class T>
	struct __RemovePointer<T *>
	{
		using Type = T;
	};
	template <class T>
	struct __RemovePointer<T *const>
	{
		using Type = T;
	};
	template <class T>
	struct __RemovePointer<T *volatile>
	{
		using Type = T;
	};
	template <class T>
	struct __RemovePointer<T *const volatile>
	{
		using Type = T;
	};
	template <typename T>
	using remove_pointer = typename __RemovePointer<T>::Type;

	template <typename T>
	struct __decay
	{
		typedef remove_cv_reference<T> type;
	};
	template <typename T>
	struct __decay<T[]>
	{
		typedef T *type;
	};
	template <typename T, decltype(sizeof(T)) N>
	struct __decay<T[N]>
	{
		typedef T *type;
	};
	// FIXME: Function decay
	template <typename T>
	using decay = typename __decay<T>::type;

	template <typename T, typename U>
	inline constexpr bool is_pointer_of_type = is_pointer<decay<U>> &&is_same<T, remove_cv<remove_pointer<decay<U>>>>;
} // namespace Vultr