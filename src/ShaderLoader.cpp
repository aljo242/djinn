#include "ShaderLoader.h"

static std::vector<char> readBinaryFile(const std::string& filename)
{
	// start at end, read as binary
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		std::string errorMessage{ "Failed to open file: " + filename };
		throw std::runtime_error(errorMessage);
	}

	size_t fileSize{ static_cast<size_t>(file.tellg()) };
	std::vector<char> buffer(fileSize);

	// go to beginning of file
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

ShaderLoader::ShaderLoader(const std::string& filename, VkDevice device, const char* name)
	:
	m_device(device),
	pName(name)
{
	code = readBinaryFile(filename);

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create shader module");
	}
}

ShaderLoader::~ShaderLoader()
{
	vkDestroyShaderModule(m_device, shaderModule, nullptr);
}
