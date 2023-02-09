#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <chrono>
#include <iostream>

#ifndef OBF
#	define OBF(x) x
#endif // !OBF

namespace FSecure::Utils
{
	/// Check if application is 64bit.
	/// @return true for x64 process.
	inline bool IsProcess64bit()
	{
#		if defined(_WIN64)
			return true;
#		elif defined(_WIN32)
			return false;
#		endif
	}

	/// Changes value to default if it is out of provided range.
	/// @param value to be clamped.
	/// @param minValue lowest accepted value.
	/// @param maxValue highest accepted value
	/// @param defaultValue default value if out of range.
	/// @return bool indicating if value was reseted to defaultValue.
	template <typename T, typename T2>
	bool IsInRange(T& value, T2 minValue, T2 maxValue, T2 defaultValue)
	{
		if (value >= minValue && value <= maxValue)
			return false;

		value = defaultValue;
		return true;
	}

	/// Align value up
	/// @param value to align
	/// @param alignment required, must be a power of 2
	/// @returns value aligned to requirement
	static inline size_t AlignValueUp(size_t value, size_t alignment)
	{
		return (value + alignment - 1) & ~(alignment - 1);
	}

	/// Generate random string.
	/// @param size of returned string.
	template <typename T = std::string, std::enable_if_t<std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>, int> = 0>
	T GenerateRandomString(size_t size)
	{
		constexpr std::string_view charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
		static std::random_device rd;
		static std::mt19937 gen(rd());
		static std::uniform_int_distribution<int> uni(0, static_cast<int>(charset.size() - 1));

		T randomString;
		randomString.resize(size);
		for (auto& e : randomString)
			e = static_cast<typename T::value_type>(charset[uni(gen)]);

		return randomString;
	}

	/// Generate random data.
	/// @param size of returned string.
	template <typename T = std::vector<uint8_t>, std::enable_if_t<std::is_same_v<T, std::string> || std::is_same_v<T, std::vector<uint8_t>>, int> = 0>
	T GenerateRandomData(size_t size)
	{
		static std::random_device rd;
		static std::mt19937_64 gen(rd());

		T randomData;
		auto alignedSize = AlignValueUp(size, sizeof(uint64_t));
		randomData.resize(alignedSize);
		for (size_t i = 0; i < alignedSize / sizeof(uint64_t); ++i)
			reinterpret_cast<uint64_t&>(randomData[sizeof(uint64_t) * i]) = gen();

		randomData.resize(size);
		return randomData;
	}

	/// Generate random unsigned int value.
	/// @param rangeFrom minimal allowed value.
	/// @param rangeTo maximal allowed value.
	/// @return random value.
	template <typename T>
	T GenerateRandomValue(T rangeFrom = std::numeric_limits<T>::min(), T rangeTo = std::numeric_limits<T>::max())
	{
		// Based on https://stackoverflow.com/a/7560564.
		static std::random_device rd;																								//< Obtain a random number from hardware.
		static std::mt19937 eng(rd());																								//< Seed the generator.
		std::uniform_int_distribution<T> distr(rangeFrom, rangeTo);																	//< Define the range.

		return distr(eng);
	}

	/// Generate random std::chrono:duration value.
	/// @param rangeFrom minimal allowed value.
	/// @param rangeTo maximal allowed value.
	/// @return random value.
	template<typename T, typename P>
	std::chrono::duration<T, P> GenerateRandomValue(std::chrono::duration<T, P> rangeFrom, std::chrono::duration<T, P> rangeTo)
	{
		return std::chrono::duration<T, P>{ GenerateRandomValue(rangeFrom.count(), rangeTo.count()) };
	}

	/// Cast a integral value to a different integral type, checking if the cast can be performed
	/// @tparam T - type to cast to
	/// @tparam T2 - type to cast from
	/// @param in - the value to cast
	/// @returns a value in T type
	/// @throws std::out_of_range if the value cannot be represented in T type
	template <typename T, typename T2>
	std::enable_if_t<(std::is_integral_v<T>&& std::is_integral_v<T2>), T> SafeCast(T2 in)
	{
		if (in < std::numeric_limits<T>::min() && in > std::numeric_limits<T>::max())
			throw std::out_of_range{ OBF("Cast cannot be performed") };

		return static_cast<T>(in);
	}

	/// Impersonation of Y2038 problem
	inline int32_t TimeSinceEpoch()
	{
		return static_cast<int32_t>(std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now()).time_since_epoch().count());
	}

	/// Alias for float based seconds
	using FloatSeconds = std::chrono::duration<float, std::chrono::seconds::period>;

	/// Alias for double based seconds
	using DoubleSeconds = std::chrono::duration<double, std::chrono::seconds::period>;

	/// Get millisecond representation
	/// @param seconds - number of seconds
	/// @returns number of milliseconds from floating seconds
	constexpr inline std::chrono::milliseconds ToMilliseconds(float seconds)
	{
		using namespace std::chrono;
		return ceil<milliseconds>(FloatSeconds{ seconds });
	}

	/// Namespace storing internal implementation of features.
	/// Symbols declared in this namespace should not be used directly
	namespace Details
	{
		/// Check out FSecure::Utils::CanApply
		/// This basic template will be used as default if instantiation of function searching idiom failed.
		template<template<class...>class T, class, class...>
		struct CanApply : std::false_type {};

		/// Check out FSecure::Utils::CanApply.
		/// This specialized template will be used if instantiation of function searching idiom succeeded.
		template<template<class...>class T, class...Ts>
		struct CanApply<T, std::void_t<T<Ts...>>, Ts...> : std::true_type {};
	}

	/// Template way to look for functions in classes using variadic argument list.
	/// CanApply::value will be true if method was found.
	/// @example
	/// Create alias that gives type of Foo call.
	///	template<class T, class...Ts>
	///	using FooReturnType = decltype(std::declval<T>().Foo(std::declval<Ts>()...));
	///
	/// Create alias to CanApply that will be used to test if some type has method Foo.
	///	template<class T, class...Ts>
	///	using HasFoo = FSecure::Utils::CanApply<FooReturnType, T, Ts...>;
	///
	/// Test type Bar can it call foo with int, double arguments.
	/// HasFoo<Bar&, int, double>::value
	template<template<class...>class T, class...Ts>
	using CanApply = Details::CanApply<T, void, Ts...>;

	/// Function splitting string.
	///
	/// @tparam copy. Indicates should strings be copied.
	/// @param stringToBeSplitted initial string to be tokenized.
	/// @param delimiter to be searched in string.
	/// @throws std::runtime_error if delimiter empty or greatter equal to stringToBeSplitted
	/// @returns std::vector<std::string> or std::vector<std::string_view> depending on copy flag.
	template<bool copy = false>
	std::vector<std::conditional_t<copy, std::string, std::string_view>> Split(std::string_view stringToBeSplitted, std::string_view delimiter)
	{
		if (delimiter.empty() && delimiter.size() >= stringToBeSplitted.size())
			throw std::runtime_error{ OBF("Delimiter size is incorrect") };

		using ReturnType = decltype(Split<copy>(stringToBeSplitted, delimiter));
		ReturnType splittedString;
		// startIndex can overflow, endIndex is checked first, and lazy check on startIndex will avoid last empty token.
		for (size_t startIndex = 0u, endIndex = 0u; endIndex < stringToBeSplitted.size() && startIndex < stringToBeSplitted.size(); startIndex = endIndex + delimiter.size())
		{
			endIndex = stringToBeSplitted.find(delimiter, startIndex);

			// skip empty tokens when delimiter is repeated.
			if (endIndex - startIndex)
				splittedString.emplace_back(stringToBeSplitted.substr(startIndex, endIndex - startIndex));
		}

		return splittedString;
	}

	/// Function that finds a substring and replaces it with another string.
	///
	/// @tparam T - string type, must contain T::npos member.
	/// @param str - input string containing phrase to replace.
	/// @param from - substring that is to be found and replaced
	/// @param to - string that should replace one given in `from` parameter
	template<class T> 
	void ReplaceString(T& str, std::basic_string_view<typename T::value_type> from, std::basic_string_view<typename T::value_type> to)
	{
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != T::npos) {
			str.replace(start_pos, size(from), to);
			start_pos += size(to);
		}
	}

	/// Proxy to Split<true>
	///
	/// @param stringToBeSplitted initial string to be tokenized.
	/// @param delimiter to be searched in string.
	/// @throws std::runtime_error if delimiter empty or greatter equal to stringToBeSplitted
	/// @returns std::vector<std::string> tokenized string.
	inline std::vector<std::string> SplitAndCopy(std::string_view stringToBeSplitted, std::string_view delimiter)
	{
		return Split<true>(stringToBeSplitted, delimiter);
	}

	/// Template function allowing structured binding to new std::string or std::string_view variables from text stored in std::string_view.
	///
	/// auto [a,b] = ToStringArray<2>(obj);
	/// @tparam expectedSize number of words that should be returned from function.
	/// @tparam copy if true words will be copied to std::string variables.
	/// @returns std::array<std::string, expectedSize> or std::array<std::string_view, expectedSize> array of strings. Size of array is known at compile time.
	/// @throws std::runtime_error if obj does not consist at least expectedSize numer of space separated strings.
	template<size_t expectedSize, bool copy = true>
	std::array<std::conditional_t<copy, std::string, std::string_view>, expectedSize> ToStringArray(std::string_view obj)
	{
		auto splited = Split(obj, OBF(" "));
		if (splited.size() < expectedSize)
			throw std::runtime_error{ OBF("ByteVector does not consist expected number of strings") };

		using ReturnType = decltype(ToStringArray<expectedSize, copy, modifyByteView>(obj));
		ReturnType retValue;
		for (auto i = 0u; i < expectedSize; ++i)
			retValue[i] = splited[i];

		return retValue;
	}

	/// Throw runtime_error if any of forbidden character is present in the string.
	inline void DisallowChars(std::string_view word, std::string_view forbiddenChars)
	{
		if (auto pos = word.find_first_of(forbiddenChars); pos != std::string::npos)
			throw std::runtime_error{ OBF_STR("Forbidden character found: ") + word[pos] };
	}

	/// Throw runtime_error if any of forbidden character is present in any of the string.
	inline void DisallowChars(std::vector<std::string_view> words, std::string_view forbiddenChars)
	{
		for (auto word : words)
			DisallowChars(word, forbiddenChars);
	}

	/// Print a debug message to stdout.
	inline void DebugPrint(std::string msg) {
#ifdef _DEBUG
		std::cout << OBF("[DEBUG] ") << msg << std::endl;
#endif
	}
}
