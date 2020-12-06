#include <concepts>
#include <vulkan/vulkan.h>
#include <assert.h>

template <class T>
concept Integral = std::is_integral<T>::value;

#define DJINN_VK_ASSERT(stmt) (assert(stmt == VK_SUCCESS))

template <Integral T>
constexpr void DJINN_POWER_TWO();