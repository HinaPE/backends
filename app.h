#ifndef BACKENDS_APP_H
#define BACKENDS_APP_H

// Copyright (c) 2023 Xayah Hina
// MPL-2.0 license

#include "backends/api.h"

namespace Kasumi
{
class App
{
public:
	virtual void launch() final;
	virtual void inspect(const INSPECTORPtr &ptr) final { _inspecting = ptr; } // inspect a INSPECTOR class

protected:
	// main methods
	virtual void prepare() {}
	virtual void update(double dt) {}
	virtual auto quit() -> bool;

	// callbacks
	virtual void key(int key, int scancode, int action, int mods);
	virtual void mouse_button(int button, int action, int mods);
	virtual void mouse_scroll(double x_offset, double y_offset);
	virtual void mouse_cursor(double x_pos, double y_pos);

	// UI
	virtual void ui_menu();
	virtual void ui_sidebar();

public:
	struct Opt
	{
		bool running = false;
		bool wireframe = false;

		int width = 1024, height = 768;
	} _opt;
	App();

public:
	friend class Platform;
	PlatformPtr _platform;
	INSPECTORPtr _inspecting;

	HinaPE::Util::Timer _timer;
	float _last_update_time = 0.f;
};
} // namespace Kasumi

#endif //BACKENDS_APP_H
