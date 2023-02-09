#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h" // it has inner vulkan header


#include "platform.h"

#include "imgui.h"
#include "implot.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"

#include <set>
#include <fstream>
#include <optional>
#include <cstdint> // for uint32_t
#include <limits> // for std::numeric_limits
#include <algorithm> // for std::clamp

namespace Kasumi
{
}

void KasumiVulkan::Platform::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		throw std::runtime_error("failed to begin recording command buffer!");

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = swapChainExtent;

	VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) swapChainExtent.width;
	viewport.height = (float) swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = swapChainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		throw std::runtime_error("failed to record command buffer!");
};

KasumiVulkan::Platform::Platform(int width, int height, const std::string &title) : _inited(false), _width(width), _height(height), _current_window(nullptr)
{
	_add_new_window(_width, _height, title, {1.f, 1.f, 1.f});
}
void KasumiVulkan::Platform::launch(const std::shared_ptr<App> &app)
{
	app->prepare();
	add_key_callback([&](int key, int scancode, int action, int mods) // key call back
					 {
						 if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
							 glfwSetWindowShouldClose(_current_window, true);
					 });
	add_key_callback([&](int key, int scancode, int action, int mods)
					 {
						 app->key(key, scancode, action, mods);
					 });
	add_mouse_callback([&](int button, int action, int mods)
					   {
						   app->mouse_button(button, action, mods);
					   });
	add_scroll_callback([&](double x_offset, double y_offset)
						{
							app->mouse_scroll(x_offset, y_offset);
						});
	add_cursor_callback([&](double x_pos, double y_pos)
						{
							app->mouse_cursor(x_pos, y_pos);
						});
	_rendering_loop(app);
}
void KasumiVulkan::Platform::add_key_callback(std::function<void(int, int, int, int)> &&callback) { _key_callbacks.emplace_back(std::move(callback)); }
void KasumiVulkan::Platform::add_mouse_callback(std::function<void(int, int, int)> &&callback) { _mouse_callbacks.emplace_back(std::move(callback)); }
void KasumiVulkan::Platform::add_scroll_callback(std::function<void(double, double)> &&callback) { _scroll_callbacks.emplace_back(std::move(callback)); }
void KasumiVulkan::Platform::add_cursor_callback(std::function<void(double, double)> &&callback) { _cursor_callbacks.emplace_back(std::move(callback)); }

void KasumiVulkan::Platform::_add_new_window(int width, int height, const std::string &title, const std::tuple<double, double, double> &clear_color)
{
	if (!_inited)
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		_current_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

		const std::vector<const char *> deviceExtensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		// Vulkan instance create
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Kasumi";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto getRequiredExtensions = [&]() -> std::vector<const char *>
		{
			uint32_t glfwExtensionCount = 0;
			const char **glfwExtensions;
			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

			std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
			return extensions;
		};
		auto extensions = getRequiredExtensions();
		std::cout << extensions.size() << " extensions supported\n";
		for (const auto &extension: extensions)
			std::cout << '\t' << extension << '\n';
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
			throw std::runtime_error("failed to create instance!");



		// Surfaces
		if (glfwCreateWindowSurface(instance, _current_window, nullptr, &surface) != VK_SUCCESS)
			throw std::runtime_error("failed to create window surface!");



		// Pick physical device
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
		if (deviceCount == 0)
			throw std::runtime_error("failed to find GPUs with Vulkan support!");

		// -> Find queue
		struct QueueFamilyIndices
		{
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			auto isComplete() -> bool { return graphicsFamily.has_value() && presentFamily.has_value(); }
		};
		auto findQueueFamilies = [&](VkPhysicalDevice d) -> QueueFamilyIndices
		{
			// Logic to find graphics queue family
			QueueFamilyIndices indices;

			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(d, &queueFamilyCount, nullptr);
			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(d, &queueFamilyCount, queueFamilies.data());

			int i = 0;
			for (const auto &queueFamily: queueFamilies)
			{
				if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
					indices.graphicsFamily = i;

				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(d, i, surface, &presentSupport);
				if (presentSupport)
					indices.presentFamily = i;

				if (indices.isComplete())
					break;
				i++;
			}

			return indices;
		};
		// -> SwapChainDetails
		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};
		auto querySwapChainSupport = [&](VkPhysicalDevice device) -> SwapChainSupportDetails
		{
			SwapChainSupportDetails details;
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
			if (formatCount != 0)
			{
				details.formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
			}

			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
			if (presentModeCount != 0)
			{
				details.presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
			}

			return details;
		};
		auto chooseSwapSurfaceFormat = [&](const std::vector<VkSurfaceFormatKHR> &availableFormats) -> VkSurfaceFormatKHR
		{
			for (const auto &availableFormat: availableFormats)
				if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
					return availableFormat;
			return availableFormats[0];
		};
		auto chooseSwapPresentMode = [&](const std::vector<VkPresentModeKHR> &availablePresentModes) -> VkPresentModeKHR
		{
			for (const auto &availablePresentMode: availablePresentModes)
				if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
					return availablePresentMode;
			return VK_PRESENT_MODE_FIFO_KHR;
		};
		auto chooseSwapExtent = [&](const VkSurfaceCapabilitiesKHR &capabilities) -> VkExtent2D
		{
			if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
				return capabilities.currentExtent;
			else
			{
				int width, height;
				glfwGetFramebufferSize(_current_window, &width, &height);

				VkExtent2D actualExtent = {
						static_cast<uint32_t>(width),
						static_cast<uint32_t>(height)
				};

				actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
				actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

				return actualExtent;
			}
		};
		auto checkDeviceExtensionSupport = [&](const VkPhysicalDevice &device) -> bool
		{
			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

			std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

			for (const auto &extension: availableExtensions)
				requiredExtensions.erase(extension.extensionName);

			return requiredExtensions.empty();
		};
		auto isDeviceSuitable = [&](const VkPhysicalDevice &d) -> bool
		{
			QueueFamilyIndices indices = findQueueFamilies(d);
			bool extensionsSupported = checkDeviceExtensionSupport(d);

			bool swapChainAdequate = false;
			if (extensionsSupported)
			{
				SwapChainSupportDetails swapChainSupport = querySwapChainSupport(d);
				swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
			}

			return indices.isComplete() && extensionsSupported && swapChainAdequate;
		};
		for (const auto &device: devices)
		{
			if (isDeviceSuitable(device))
			{
				physicalDevice = device;
				break; // select first suitable device
			}
		}

		// Create logic device
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

		float queuePriority = 1.0f;
		for (uint32_t queueFamily: uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		VkDeviceCreateInfo createInfoLD{};
		createInfoLD.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfoLD.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfoLD.pQueueCreateInfos = queueCreateInfos.data();
		createInfoLD.pEnabledFeatures = &deviceFeatures;
		createInfoLD.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfoLD.ppEnabledExtensionNames = deviceExtensions.data();
		createInfoLD.enabledLayerCount = 0; // dont use layer yet

		if (vkCreateDevice(physicalDevice, &createInfoLD, nullptr, &device) != VK_SUCCESS)
			throw std::runtime_error("failed to create logical device!");

		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);

		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);



		// Create swap chain
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 /* where 0 is a special value that means that there is no maximum: */ && imageCount > swapChainSupport.capabilities.maxImageCount)
			imageCount = swapChainSupport.capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR createInfoSC{};
		createInfoSC.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfoSC.surface = surface;

		createInfoSC.minImageCount = imageCount;
		createInfoSC.imageFormat = surfaceFormat.format;
		createInfoSC.imageColorSpace = surfaceFormat.colorSpace;
		createInfoSC.imageExtent = extent;
		createInfoSC.imageArrayLayers = 1;
		createInfoSC.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indicesForSC = findQueueFamilies(physicalDevice);
		uint32_t queueFamilyIndicesForSC[] = {indicesForSC.graphicsFamily.value(), indicesForSC.presentFamily.value()};
		if (indicesForSC.graphicsFamily != indicesForSC.presentFamily)
		{
			createInfoSC.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfoSC.queueFamilyIndexCount = 2;
			createInfoSC.pQueueFamilyIndices = queueFamilyIndicesForSC;
		} else
		{
			createInfoSC.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfoSC.queueFamilyIndexCount = 0; // Optional
			createInfoSC.pQueueFamilyIndices = nullptr; // Optional
		}
		createInfoSC.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfoSC.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfoSC.presentMode = presentMode;
		createInfoSC.clipped = VK_TRUE;
		createInfoSC.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device, &createInfoSC, nullptr, &swapChain) != VK_SUCCESS)
			throw std::runtime_error("failed to create swap chain!");

		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());


		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;



		// Image view
		swapChainImageViews.resize(swapChainImages.size());
		for (size_t i = 0; i < swapChainImages.size(); i++)
		{
			VkImageViewCreateInfo createInfoIV{};
			createInfoIV.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfoIV.image = swapChainImages[i];
			createInfoIV.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfoIV.format = swapChainImageFormat;
			createInfoIV.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfoIV.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfoIV.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfoIV.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfoIV.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfoIV.subresourceRange.baseMipLevel = 0;
			createInfoIV.subresourceRange.levelCount = 1;
			createInfoIV.subresourceRange.baseArrayLayer = 0;
			createInfoIV.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device, &createInfoIV, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
				throw std::runtime_error("failed to create image views!");
		}



		// Render pass
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
			throw std::runtime_error("failed to create render pass!");

		// Graphics pipeline

		// Shaders

		std::string vertex_path = std::string(BuiltinShaderDir) + "vulkan_v.spv";
		std::string fragment_path = std::string(BuiltinShaderDir) + "vulkan_f.spv";
		std::ifstream vertex_shader(vertex_path, std::ios::ate | std::ios::binary);
		if (!vertex_shader.is_open())
			throw std::runtime_error("failed to open vertex shader!");

		size_t fileSize = (size_t) vertex_shader.tellg();
		std::vector<char> vertex_buffer(fileSize);
		vertex_shader.seekg(0);
		vertex_shader.read(vertex_buffer.data(), fileSize);
		vertex_shader.close();

		std::ifstream fragment_shader(fragment_path, std::ios::ate | std::ios::binary);
		if (!fragment_shader.is_open())
			throw std::runtime_error("failed to open fragment shader!");

		fileSize = (size_t) fragment_shader.tellg();
		std::vector<char> fragment_buffer(fileSize);
		fragment_shader.seekg(0);
		fragment_shader.read(fragment_buffer.data(), fileSize);
		fragment_shader.close();

		auto createShaderModule = [&](const std::vector<char> &code) -> VkShaderModule
		{
			VkShaderModuleCreateInfo createInfoSM{};
			createInfoSM.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfoSM.codeSize = code.size();
			createInfoSM.pCode = reinterpret_cast<const uint32_t *>(code.data());

			VkShaderModule shaderModule;
			if (vkCreateShaderModule(device, &createInfoSM, nullptr, &shaderModule) != VK_SUCCESS)
				throw std::runtime_error("failed to create shader module!");

			return shaderModule;
		};

		VkShaderModule vertShaderModule = createShaderModule(vertex_buffer);
		VkShaderModule fragShaderModule = createShaderModule(fragment_buffer);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

		// -> Fixed Funtions
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		std::vector<VkDynamicState> dynamicStates = {
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pushConstantRangeCount = 0;

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("failed to create pipeline layout!");

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
			throw std::runtime_error("failed to create graphics pipeline!");

		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroyShaderModule(device, vertShaderModule, nullptr);



		// Framebuffer
		swapChainFramebuffers.resize(swapChainImageViews.size());
		for (size_t i = 0; i < swapChainImageViews.size(); i++)
		{
			VkImageView attachments[] = {
					swapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
				throw std::runtime_error("failed to create framebuffer!");
		}



		// Command buffer
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
			throw std::runtime_error("failed to create command pool!");

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate command buffers!");


		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS)
			throw std::runtime_error("failed to create synchronization objects for a frame!");
	}
}

void KasumiVulkan::Platform::_rendering_loop(const std::shared_ptr<App> &app)
{
	while (!glfwWindowShouldClose(_current_window) || app->quit())
	{
		_begin_frame();
		app->update(0.02);

		vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
		vkResetFences(device, 1, &inFlightFence);

		uint32_t imageIndex;
		vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

		vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
		recordCommandBuffer(commandBuffer, imageIndex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
		VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = {swapChain};
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		vkQueuePresentKHR(presentQueue, &presentInfo);

		_end_frame();
	}
}
void KasumiVulkan::Platform::_clear_window()
{
}
void KasumiVulkan::Platform::_begin_frame()
{
	_clear_window();
//	ImGui_ImplVulkan_NewFrame();
//	ImGui_ImplGlfw_NewFrame();
//	ImGui::NewFrame();
}
void KasumiVulkan::Platform::_end_frame()
{
//	ImGui::Render();
//	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(_current_window);
	glfwPollEvents();
}
KasumiVulkan::App::App(int width, int height, const std::string &title) : _platform(std::make_shared<KasumiVulkan::Platform>(width, height, title)), _width(width), _height(height) {}
void KasumiVulkan::App::launch() { _platform->launch(shared_from_this()); }
