#include "shader.h"
#include "vulkan/vulkan.h"

#include <fstream>

//namespace Kasumi
//{
//extern VkDevice device;
//}

KasumiVulkan::Shader::Shader(const std::string &vertex_path, const std::string &fragment_path) : Shader(vertex_path, fragment_path, "") {}
KasumiVulkan::Shader::Shader(const std::string &vertex_path, const std::string &fragment_path, const std::string &geometry_path)
{
//	std::ifstream vertex_shader(vertex_path, std::ios::ate | std::ios::binary);
//	if (!vertex_shader.is_open())
//		throw std::runtime_error("failed to open vertex shader!");
//
//	size_t fileSize = (size_t) vertex_shader.tellg();
//	std::vector<char> vertex_buffer(fileSize);
//	vertex_shader.seekg(0);
//	vertex_shader.read(vertex_buffer.data(), fileSize);
//	vertex_shader.close();
//
//	std::ifstream fragment_shader(fragment_path, std::ios::ate | std::ios::binary);
//	if (!fragment_shader.is_open())
//		throw std::runtime_error("failed to open fragment shader!");
//
//	fileSize = (size_t) fragment_shader.tellg();
//	std::vector<char> fragment_buffer(fileSize);
//	fragment_shader.seekg(0);
//	fragment_shader.read(fragment_buffer.data(), fileSize);
//	fragment_shader.close();
//
//	auto createShaderModule = [&](const std::vector<char> &code) -> VkShaderModule
//	{
//		VkShaderModuleCreateInfo createInfo{};
//		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
//		createInfo.codeSize = code.size();
//		createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
//
//		VkShaderModule shaderModule;
//		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
//			throw std::runtime_error("failed to create shader module!");
//
//		return shaderModule;
//	};
//
//	VkShaderModule vertShaderModule = createShaderModule(vertex_buffer);
//	VkShaderModule fragShaderModule = createShaderModule(fragment_buffer);
//
//	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
//	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
//	vertShaderStageInfo.module = vertShaderModule;
//	vertShaderStageInfo.pName = "main";
//
//	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
//	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
//	fragShaderStageInfo.module = fragShaderModule;
//	fragShaderStageInfo.pName = "main";
//
//	VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
}
KasumiVulkan::Shader::Shader(const char *vertex_src, const char *fragment_src) : Shader(vertex_src, fragment_src, nullptr) {}
KasumiVulkan::Shader::Shader(const char *vertex_src, const char *fragment_src, const char *geometry_src) {}
KasumiVulkan::Shader::~Shader() {}
void KasumiVulkan::Shader::use() const {}
void KasumiVulkan::Shader::uniform(const std::string &name, bool value) const {}
void KasumiVulkan::Shader::uniform(const std::string &name, int value) const {}
void KasumiVulkan::Shader::uniform(const std::string &name, unsigned int value) const {}
void KasumiVulkan::Shader::uniform(const std::string &name, float value) const {}
void KasumiVulkan::Shader::uniform(const std::string &name, const mVector2 &value) const {}
void KasumiVulkan::Shader::uniform(const std::string &name, const mVector3 &value) const {}
void KasumiVulkan::Shader::uniform(const std::string &name, const mVector4 &value) const {}
void KasumiVulkan::Shader::uniform(const std::string &name, mMatrix3x3 value) const {}
void KasumiVulkan::Shader::uniform(const std::string &name, mMatrix4x4 value) const {}

auto KasumiVulkan::Shader::validate(unsigned int id) -> bool
{
	return true;
}
