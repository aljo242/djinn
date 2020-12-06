#ifndef CORE_H
#define CORE_H

#include <concepts>
#include <vulkan/vulkan.h>
#include <assert.h>

//template <class T>
//concept Integral = std::is_integral<T>::value;

#define DJINN_VK_ASSERT(stmt) (assert(stmt == VK_SUCCESS))

template <std::integral T>
constexpr void DJINN_POWER_TWO(T value)
{
	static_assert((value) & (value - 1) == 0);
}

#define DJINN_UNUSED(expr) (void)(expr)

#endif