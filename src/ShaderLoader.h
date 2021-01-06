#ifndef SHADER_LOADER_H
#define SHADER_LOADER_H

#include <vector>
#include <fstream>

#include <vulkan/vulkan.h>

struct ShaderLoaderCreateInfo
{
	std::string filename;
	VkShaderStageFlagBits stage;
};

class ShaderLoader
{
public:
	ShaderLoader(const std::string& filename, VkDevice device, const VkShaderStageFlagBits stageFlag);
	ShaderLoader(const ShaderLoaderCreateInfo& createInfo, VkDevice device);
	void DestroyModule();

	VkShaderModule shaderModule{VK_NULL_HANDLE};
	VkShaderStageFlagBits stage;
	const char* pName = "main";

private:
	std::vector<char> code;
	VkDevice m_device{VK_NULL_HANDLE};
};

#endif