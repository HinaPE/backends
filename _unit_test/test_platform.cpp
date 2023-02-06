#include "platform.h"
#include "model.h"

class TestApp : public Kasumi::App
{
private:
	Kasumi::ModelPtr _model;

public:
	TestApp(int width, int height, const std::string &title) : App(width, height, title) {}

	void prepare() final
	{
		_model = std::make_shared<Kasumi::Model>(std::string(BuiltinModelDir) + "bunny.obj");
	}
	void update(double dt) final
	{
		_model->render();
	}
	auto quit() -> bool final { return App::quit(); }
	void key(int key, int scancode, int action, int mods) final { App::key(key, scancode, action, mods); }
	void mouse_button(int button, int action, int mods) final { App::mouse_button(button, action, mods); }
	void mouse_scroll(double x_offset, double y_offset) final { App::mouse_scroll(x_offset, y_offset); }
	void mouse_cursor(double x_pos, double y_pos) final { App::mouse_cursor(x_pos, y_pos); }
};

auto main() -> int
{
	auto test = std::make_shared<TestApp>(800, 600, "Test App");
	test->launch();
	return 0;
}
