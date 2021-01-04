#ifdef _MSC_VER

#pragma warning(push)
#pragma warning(disable : 26812) // disabling a warning when including a header works normally for most warnings.

// caused glfw to automatically include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_SIMD_AVX2
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#pragma warning(disable : 26812) // disabling a warning when including a header works normally for most warnings.

#pragma warning(pop)

#endif