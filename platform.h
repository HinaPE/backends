#ifndef KASUMI_PLATFORM_H
#define KASUMI_PLATFORM_H

// Copyright (c) 2023 Xayah Hina
// MPL-2.0 license

// Dependency:
// - Fully Decoupled

#include <string>
#include <utility>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <tuple>

namespace Kasumi
{
class App;
class Platform
{
public:
	// methods
	void launch(App &app);

	// callbacks
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

		bool MSAA = true;
		int MSAA_sample = 4;
	} _opt;
	Platform(int width, int height, const std::string &title = "Kasumi: illumine the endless night");

public:
	Platform(const Platform &) = delete;
	Platform(Platform &&) = delete;
	~Platform() = default;
	auto operator=(const Platform &) -> Platform & = delete;
	auto operator=(Platform &&) -> Platform & = delete;

private:
	void _add_new_window(int width, int height, const std::string &title, const std::tuple<double, double, double> &clear_color);
	void _rendering_loop(App &app);
	void _clear_window();
	void _begin_frame();
	void _end_frame();

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
class App
{
public:
	virtual void launch() final;

protected:
	// main methods
	virtual void prepare() {}
	virtual void update(double dt) {}
	virtual auto quit() -> bool { return false; }

	// callbacks
	virtual void key(int key, int scancode, int action, int mods) {}
	virtual void mouse_button(int button, int action, int mods) {}
	virtual void mouse_scroll(double x_offset, double y_offset) {}
	virtual void mouse_cursor(double x_pos, double y_pos) {}

	// UI
	virtual void ui_menu();
	virtual void ui_sidebar();

public:
	struct Opt
	{
		bool running = false;

		int width = 1024;
		int height = 768;
	} _opt;
	explicit App(const Opt &opt);

public:
	friend class Platform;
	PlatformPtr _platform;
};
} // namespace Kasumi

#endif //KASUMI_PLATFORM_H
