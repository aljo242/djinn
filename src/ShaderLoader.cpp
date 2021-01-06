#include "ShaderLoader.h"
#include "core/core.h"

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

ShaderLoader::ShaderLoader(const std::string& filename, VkDevice device, const VkShaderStageFlagBits stageFlag)
	:
	m_device(device),
	stage(stageFlag)
{
	code = readBinaryFile(filename);

	VkShaderModuleCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = code.size();
	info.pCode = reinterpret_cast<const uint32_t*>(code.data());

	const auto result = vkCreateShaderModule(device, &info, nullptr, &shaderModule);
	DJINN_VK_ASSERT(result);
}


ShaderLoader::ShaderLoader(const ShaderLoaderCreateInfo& createInfo, VkDevice device)
	:
	m_device(device),
	stage(createInfo.stage)
{
	code = readBinaryFile(createInfo.filename);

	VkShaderModuleCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = code.size();
	info.pCode = reinterpret_cast<const uint32_t*>(code.data());

	const auto result = vkCreateShaderModule(device, &info, nullptr, &shaderModule);
	DJINN_VK_ASSERT(result);
}

void ShaderLoader::DestroyModule()
{
	vkDestroyShaderModule(m_device, shaderModule, nullptr);
}
