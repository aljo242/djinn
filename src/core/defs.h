#ifndef DEFS_INCLUDE_H
#define DEFS_INCLUDE_H

#include <string>
#include <vector>

constexpr uint32_t INITIAL_WIN_WIDTH{ 1280 };
constexpr uint32_t INITIAL_WIN_HEIGHT{ 720 };

const std::vector<const char*> VALIDATION_LAYERS { "VK_LAYER_KHRONOS_validation" };
const std::vector<const char*> DEVICE_EXTENSIONS{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#if defined(_DEBUG)
constexpr bool ENABLE_VALIDATION_LAYERS{ true };
#else
constexpr bool ENABLE_VALIDATION_LAYERS{ false };
#endif

// #define IMGUI_UNLIMITED_FRAME_RATE
#if defined(_DEBUG)
#define IMGUI_VULKAN_DEBUG_REPORT
#endif

const std::string APP_NAME{ "Hello Triangle" };
constexpr uint32_t APP_VERSION{ VK_MAKE_VERSION(1, 0, 0) };
const std::string ENGINE_NAME{ "Djinn" };
constexpr uint32_t ENGINE_VERSION{ VK_MAKE_VERSION(1, 0, 0) };


const std::string MODEL_PATH{ "res/model/viking_room.obj" };
const std::string TEXTURE_PATH{ "res/tex/viking_room.png" };

#endif