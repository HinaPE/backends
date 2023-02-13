#include "GLFW/glfw3.h"

#include "platform.h"
#include "model.h"

class TestApp : public Kasumi::App
{
private:
	Kasumi::ModelPtr _model;
	Kasumi::UniversalMeshPtr _mesh;
	Kasumi::LinesPtr _lines;
	mMatrix4x4 projection;
	mMatrix4x4 view;
	real x = 0;
	real z = 2;

public:
	TestApp(int width, int height, const std::string &title) : App(width, height, title) {}

	void prepare() final
	{
		Kasumi::Shader::Init();

//		_model = std::make_shared<Kasumi::Model>(std::string(BuiltinModelDir) + "bunny.obj");
		_mesh = std::make_shared<Kasumi::UniversalMesh>("cube", HinaPE::Color::RED);

		_lines = std::make_shared<Kasumi::Lines>();
		_lines->add({-0.9, 0, 0}, {0.9, 0, 0});
	}
	void update(double dt) final
	{
//		_model->render();
//		_lines->render();

		projection = Kasumi::Camera::project_matrix(45, 1, 0.1, 100);
		view = Kasumi::Camera::view_matrix({x, 0, z}, {1, 0, 0, 0});

		{
			auto &shader = Kasumi::Shader::DefaultLineShader;
			shader->use();
			shader->uniform("projection", projection);
			shader->uniform("view", view);
			shader->uniform("model", mMatrix4x4::Identity());
			_lines->render(*shader);
		}

		{
			auto &shader = Kasumi::Shader::DefaultMeshShader;
			shader->use();
			shader->uniform("projection", projection);
			shader->uniform("view", view);
			shader->uniform("model", mMatrix4x4::Identity());
			_mesh->render(*shader);
		}
	}
	auto quit() -> bool final { return App::quit(); }
	void key(int key, int scancode, int action, int mods) final
	{
		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
			x += 0.1f;
		else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
			x -= 0.1f;
		else if (key == GLFW_KEY_UP && action == GLFW_PRESS)
			z -= 0.1f;
		else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
			z += 0.1f;
	}
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
