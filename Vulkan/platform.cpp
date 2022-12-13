#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h" // it has inner vulkan header
//#include "vulkan/vulkan.h"

#include "platform.h"

#include "imgui.h"
#include "implot.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"

#include <set>
#include <cstdint> // for uint32_t
#include <limits> // for std::numeric_limits
#include <algorithm> // for std::clamp

namespace Kasumi
{
struct VulkanClass
{
	VkInstance instance;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
} _vulkan;
}

KasumiVulkan::Platform::Platform(int width, int height, const std::string &title) : _inited(false), _width(width), _height(height), _current_window(nullptr)
{
	add_new_window(_width, _height, title, {1.f, 1.f, 1.f});
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
	rendering_loop(app);
}
void KasumiVulkan::Platform::add_key_callback(std::function<void(int, int, int, int)> &&callback) { _key_callbacks.emplace_back(std::move(callback)); }
void KasumiVulkan::Platform::add_mouse_callback(std::function<void(int, int, int)> &&callback) { _mouse_callbacks.emplace_back(std::move(callback)); }
void KasumiVulkan::Platform::add_scroll_callback(std::function<void(double, double)> &&callback) { _scroll_callbacks.emplace_back(std::move(callback)); }
void KasumiVulkan::Platform::add_cursor_callback(std::function<void(double, double)> &&callback) { _cursor_callbacks.emplace_back(std::move(callback)); }

void KasumiVulkan::Platform::add_new_window(int width, int height, const std::string &title, const std::tuple<double, double, double> &clear_color)
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
		VkInstance &instance = Kasumi::_vulkan.instance;
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



		//
		VkSurfaceKHR surface = Kasumi::_vulkan.surface;
		if (glfwCreateWindowSurface(instance, _current_window, nullptr, &surface) != VK_SUCCESS)
			throw std::runtime_error("failed to create window surface!");



		// Pick physical device
		VkPhysicalDevice physicalDevice = Kasumi::_vulkan.physicalDevice;
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
		VkDevice device = Kasumi::_vulkan.device;

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

		VkQueue graphicsQueue = Kasumi::_vulkan.graphicsQueue;
		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);

		VkQueue presentQueue = Kasumi::_vulkan.presentQueue;
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

		VkSwapchainKHR swapChain = Kasumi::_vulkan.swapChain;
		if (vkCreateSwapchainKHR(device, &createInfoSC, nullptr, &swapChain) != VK_SUCCESS)
			throw std::runtime_error("failed to create swap chain!");

		std::vector<VkImage> &swapChainImages = Kasumi::_vulkan.swapChainImages;
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

		VkFormat &swapChainImageFormat = Kasumi::_vulkan.swapChainImageFormat;
		VkExtent2D &swapChainExtent = Kasumi::_vulkan.swapChainExtent;

		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;



		// Image view
		std::vector<VkImageView> &swapChainImageViews = Kasumi::_vulkan.swapChainImageViews;
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



		// Graphics pipeline
		
	}
}

void KasumiVulkan::Platform::rendering_loop(const std::shared_ptr<App> &app)
{
	while (!glfwWindowShouldClose(_current_window) || app->quit())
	{
		begin_frame();
		app->update(0.02);
		end_frame();
	}
}
void KasumiVulkan::Platform::clear_window()
{
}
void KasumiVulkan::Platform::begin_frame()
{
	clear_window();
//	ImGui_ImplVulkan_NewFrame();
//	ImGui_ImplGlfw_NewFrame();
//	ImGui::NewFrame();
}
void KasumiVulkan::Platform::end_frame()
{
//	ImGui::Render();
//	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(_current_window);
	glfwPollEvents();
}
KasumiVulkan::App::App(int width, int height, const std::string &title) : _platform(std::make_shared<KasumiVulkan::Platform>(width, height, title)), _width(width), _height(height) {}
void KasumiVulkan::App::launch() { _platform->launch(shared_from_this()); }
