#include "ShaderLoader.h"

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
