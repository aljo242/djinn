#ifndef DEFS_INCLUDE_H
#define DEFS_INCLUDE_H

constexpr uint32_t INITIAL_WIN_WIDTH{ 1280 };
constexpr uint32_t INITIAL_WIN_HEIGHT{ 720 };

const std::vector<const char*> VALIDATION_LAYERS { "VK_LAYER_KHRONOS_validation" };
const std::vector<const char*> DEVICE_EXTENSIONS{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#if defined(_DEBUG)
constexpr bool ENABLE_VALIDATION_LAYERS{ true };
#else
constexpr bool ENABLE_VALIDATION_LAYERS{ false };
#endif


const char* APP_NAME{ "Hello Triangle" };
constexpr uint32_t APP_VERSION{ VK_MAKE_VERSION(1, 0, 0) };
const char* ENGINE_NAME{ "Djinn" };
constexpr uint32_t ENGINE_VERSION{ VK_MAKE_VERSION(1, 0, 0) };

#endif