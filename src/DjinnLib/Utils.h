#ifndef DJINNLIB_UTILS_INCLUDE_H
#define DJINNLIB_UTILS_INCLUDE_H

#include <concepts>

namspace Djinn
{
	template<std::unsigned_integral UINT_T>
	constexpr UINT_T BIT(const UINT_T uint_t)
	{
		return (static_cast<UINT_T>(1ULL << (uint_t)));
	}

	template<std::unsigned_integral UINT_T>
	constexpr UINT_T SIZE_KB(const UINT_T uint_t)
	{
		return (static_cast<UINT_T>(((uint_t) + 1023) / 1024 );
	}

	template<std::unsigned_integral UINT_T>
	constexpr UINT_T SIZE_MB(const UINT_T uint_t)
	{
		return (static_cast<UINT_T>((SIZE_KB(uint_t)+1023) / 1024);
	}

	template<std::unsigned_integral UINT_T>
	constexpr UINT_T SIZE_GB(const UINT_T uint_t)
	{
		return (static_cast<UINT_T>((SIZE_MB(uint_t)+1023) / 1024);
	}

}

#endif // DJINNLIB_UTILS_INCLUDE_H