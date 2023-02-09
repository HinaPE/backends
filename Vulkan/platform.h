#ifndef KASUMI_PLATFORM_H
#define KASUMI_PLATFORM_H
#include "vulkan/vulkan.h"
// This is a Vulkan Implementation of Platform

#include <string>
#include <utility>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <tuple>
#include <iostream>

namespace KasumiVulkan
{
class App;
class Platform
{
public: //! ==================== Public Methods ====================
	void _add_new_window(int width, int height, const std::string &title, const std::tuple<double, double, double> &clear_color);
	void add_key_callback(std::function<void(int key, int scancode, int action, int mods)> &&callback);
	void add_mouse_callback(std::function<void(int button, int action, int mods)> &&callback);
	void add_scroll_callback(std::function<void(double x_offset, double y_offset)> &&callback);
	void add_cursor_callback(std::function<void(double x_pos, double y_pos)> &&callback);
	void launch(const std::shared_ptr<App> &app);

public: //! ==================== Platform Opt ====================
	struct Opt
	{
		bool clear_color = true;
		bool clear_depth = true;
		bool clear_stencil = true;

		bool MSAA = true;
		int MSAA_sample = 4;
	} _opt;

//! ==================== Constructors & Destructor ====================
//! - [DELETE] copy constructor & copy assignment operator
//! - [DELETE] move constructor & move assignment operator
public:
	Platform(int width, int height, const std::string& title = "Kasumi: illumine the endless night");
	Platform(const Platform &) = delete;
	Platform(Platform &&) = delete;
	~Platform() = default;
	auto operator=(const Platform &) -> Platform & = delete;
	auto operator=(Platform &&) -> Platform & = delete;

private:
	void _rendering_loop(const std::shared_ptr<App> &app);
	void _clear_window();
	void _begin_frame();
	void _end_frame();

	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

private:
	bool _inited;
	int _width, _height;
	std::string _current_window_name;
	class GLFWwindow *_current_window;
	std::map<std::string, GLFWwindow *> _windows;
	std::map<std::string, std::tuple<double, double, double>> _clear_colors;
	std::vector<std::function<void(int, int, int, int)>> _key_callbacks;
	std::vector<std::function<void(int, int, int)>> _mouse_callbacks;
	std::vector<std::function<void(double, double)>> _scroll_callbacks;
	std::vector<std::function<void(double, double)>> _cursor_callbacks;

private:
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
	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkCommandPool commandPool;
	VkCommandBuffer commandBuffer;

	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkFence inFlightFence;
};
using PlatformPtr = std::shared_ptr<Platform>;
class App : public std::enable_shared_from_this<App>
{
public: //! ==================== Abstract Interfaces ====================
	virtual void prepare() {}
	virtual void update(double dt) {}
	virtual auto quit() -> bool { return false; }
	virtual void key(int key, int scancode, int action, int mods) {}
	virtual void mouse_button(int button, int action, int mods) {}
	virtual void mouse_scroll(double x_offset, double y_offset) {}
	virtual void mouse_cursor(double x_pos, double y_pos) {}
	virtual void launch() final;

public: //! ==================== Constructors ====================
	App(int width, int height, const std::string &title = "Kasumi Renderer");

protected:
	int _width, _height;

private:
	PlatformPtr _platform; // the platform that this app is running on
};
}

#endif //KASUMI_PLATFORM_H
