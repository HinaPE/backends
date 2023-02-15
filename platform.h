#ifndef KASUMI_PLATFORM_H
#define KASUMI_PLATFORM_H

// Copyright (c) 2023 Xayah Hina
// MPL-2.0 license

// Dependency:
// - Shader

#include <string>
#include <utility>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <tuple>
#include <array>

namespace Kasumi
{
class App;
class Platform
{
public:
	void launch(App &app);

public:
	struct Opt
	{
		bool clear_color = true;
		bool clear_depth = true;
		bool clear_stencil = true;

		bool MSAA = true;
		int MSAA_sample = 4;

		std::array<float, 3> background_color = {1.f, 1.f, 1.f};
	} _opt;
	Platform(int width, int height, const std::string &title = "Kasumi: illumine the endless night");

public:
	Platform(const Platform &) = delete;
	Platform(Platform &&) = delete;
	~Platform() = default;
	auto operator=(const Platform &) -> Platform & = delete;
	auto operator=(Platform &&) -> Platform & = delete;

private:
	void _new_window(int width, int height, const std::string &title);
	void _rendering_loop(App &app);
	void _clear_window();
	void _begin_frame();
	void _end_frame();

private:
	bool _inited;
	int _width, _height;
	std::string _current_window_name;
	class GLFWwindow *_current_window;
	std::vector<std::function<void(int, int, int, int)>> _key_callbacks;
	std::vector<std::function<void(int, int, int)>> _mouse_callbacks;
	std::vector<std::function<void(double, double)>> _scroll_callbacks;
	std::vector<std::function<void(double, double)>> _cursor_callbacks;
};
using PlatformPtr = std::shared_ptr<Platform>;
} // namespace Kasumi
#endif //KASUMI_PLATFORM_H
