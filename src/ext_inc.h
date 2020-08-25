#ifdef _MSC_VER

#pragma warning(push)
#pragma warning(disable : 26812) // disabling a warning when including a header works normally for most warnings.

// caused glfw to automatically include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#pragma warning(disable : 26812) // disabling a warning when including a header works normally for most warnings.

#pragma warning(pop)

#endif