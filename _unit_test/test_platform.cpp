#include "platform.h"
#include "model.h"

class TestApp : public Kasumi::App
{
private:
	Kasumi::ModelPtr _model;
	Kasumi::LinesPtr _lines;

public:
	TestApp(int width, int height, const std::string &title) : App(width, height, title) {}

	void prepare() final
	{
		Kasumi::Shader::Init();

//		_model = std::make_shared<Kasumi::Model>(std::string(BuiltinModelDir) + "bunny.obj");

		_lines = std::make_shared<Kasumi::Lines>();
		_lines->add({-0.5, 0, 0}, {0.5, 0, 0});
	}
	void update(double dt) final
	{
//		_model->render();
//		_lines->render();

		auto &shader = Kasumi::Shader::DefaultLineShader;
		shader->use();
		shader->uniform("projection", mMatrix4x4::Identity());
		shader->uniform("view", Kasumi::Camera::view_matrix({0, 0, 2}, {1, 0, 0, 0}));
		shader->uniform("model", mMatrix4x4::Identity());
		_lines->render(*Kasumi::Shader::DefaultLineShader);
	}
	auto quit() -> bool final { return App::quit(); }
	void key(int key, int scancode, int action, int mods) final { App::key(key, scancode, action, mods); }
	void mouse_button(int button, int action, int mods) final { App::mouse_button(button, action, mods); }
	void mouse_scroll(double x_offset, double y_offset) final { App::mouse_scroll(x_offset, y_offset); }
	void mouse_cursor(double x_pos, double y_pos) final { App::mouse_cursor(x_pos, y_pos); }
};

auto main() -> int
{
	auto test = std::make_shared<TestApp>(1024, 768, "Test App");
	test->launch();
	return 0;
}
