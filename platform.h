#ifndef KASUMI_PLATFORM_H
#define KASUMI_PLATFORM_H


#include <string>
#include <utility>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <tuple>
#include <iostream>

namespace Kasumi
{
class App;
class Platform
{
public:
	void launch(const std::shared_ptr<App> &app);
	void add_key_callback(std::function<void(int key, int scancode, int action, int mods)> &&callback);
	void add_mouse_callback(std::function<void(int button, int action, int mods)> &&callback);
	void add_scroll_callback(std::function<void(double x_offset, double y_offset)> &&callback);
	void add_cursor_callback(std::function<void(double x_pos, double y_pos)> &&callback);

public:
	struct Opt
	{
		bool clear_color = true;
		bool clear_depth = true;
		bool clear_stencil = true;
	};
	Opt opt;

public:
	Platform(int width, int height);
	Platform(const Platform &) = delete;
	Platform(Platform &&) = delete;
	~Platform() = default;
	auto operator=(const Platform &) -> Platform & = delete;
	auto operator=(Platform &&) -> Platform & = delete;

private:
	void add_new_window(int width, int height, const std::string &title, const std::tuple<double, double, double> &clear_color);
	void rendering_loop(const std::shared_ptr<App> &app);
	void clear_window();
	void begin_frame();
	void end_frame();

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
};
using PlatformPtr = std::shared_ptr<Platform>;
class App : public std::enable_shared_from_this<App>
{
public:
	virtual void prepare() = 0;
	virtual void update(double dt) = 0;
	virtual auto quit() -> bool = 0;
	virtual void key(int key, int scancode, int action, int mods) {}
	virtual void mouse_button(int button, int action, int mods) {}
	virtual void mouse_scroll(double x_offset, double y_offset) {}
	virtual void mouse_cursor(double x_pos, double y_pos) {}

public:
	App();
	App(int width, int height, const std::string &title = "Kasumi Renderer");
	virtual void launch() final;

private:
	PlatformPtr _platform;
};
}

#endif //KASUMI_PLATFORM_H
