#ifndef SHADER_LOADER_H
#define SHADER_LOADER_H

#include <vector>
#include <fstream>

#include <vulkan/vulkan.h>

class ShaderLoader
{
public:
	ShaderLoader(const std::string& filename, VkDevice device, const char* name = "main");
	~ShaderLoader();

	VkShaderModule shaderModule{VK_NULL_HANDLE};
	const char* pName;

private:
	std::vector<char> code;
	VkDevice m_device{VK_NULL_HANDLE};
};

#endif