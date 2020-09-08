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
	VkDevice m_device;
};


static std::vector<char> readBinaryFile(const std::string& filename)
{
	// start at end, read as binary
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		std::string errorMessage {"Failed to open file: " + filename};
		throw std::runtime_error(errorMessage);
	}

	size_t fileSize			{static_cast<size_t>(file.tellg())};
	std::vector<char> buffer(fileSize);

	// go to beginning of file
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

#endif