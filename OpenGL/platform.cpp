#include "glad/glad.h" // include glad before glfw
#include "GLFW/glfw3.h"
#include "../api.h"

#include "imgui.h"
#include "implot.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"
#include "../font.dat"
#include "backends/platform.h"


#include <stdexcept>

Kasumi::Platform::Platform(int width, int height, const std::string &title) : _inited(false), _width(width), _height(height), _current_window(nullptr)
{
	_new_window(_width, _height, title);
}

void Kasumi::Platform::launch(App &app)
{
	app.prepare();
	_key_callbacks.emplace_back([&](int key, int scancode, int action, int mods) // key call back
								{
									if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
										glfwSetWindowShouldClose(_current_window, true);
									if (key == GLFW_KEY_C && action == GLFW_PRESS)
										_opt.show_color_picker = !_opt.show_color_picker;
									if (key == GLFW_KEY_T && action == GLFW_PRESS)
										_opt.show_benchmark = !_opt.show_benchmark;
								});
	_key_callbacks.emplace_back([&](int key, int scancode, int action, int mods)
								{
									app.key(key, scancode, action, mods);
								});
	_mouse_callbacks.emplace_back([&](int button, int action, int mods)
								  {
									  app.mouse_button(button, action, mods);
								  });
	_scroll_callbacks.emplace_back([&](double x_offset, double y_offset)
								   {
									   app.mouse_scroll(x_offset, y_offset);
								   });
	_cursor_callbacks.emplace_back([&](double x_pos, double y_pos)
								   {
									   app.mouse_cursor(x_pos, y_pos);
								   });
	_rendering_loop(app);
}
void Kasumi::Platform::_new_window(int width, int height, const std::string &title)
{
	if (!_inited)
	{
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		if (_opt.MSAA)
			glfwWindowHint(GLFW_SAMPLES, _opt.MSAA_sample);
#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	}

	_current_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	_current_window_name = title;
	if (_current_window == nullptr)
		throw std::runtime_error("Failed to create GLFW window");
	glfwMakeContextCurrent(_current_window);
	glfwSetWindowUserPointer(_current_window, this);
	glfwSetFramebufferSizeCallback(_current_window, [](GLFWwindow *window, int width, int height) { glViewport(0, 0, width, height); });
	glfwSetKeyCallback(_current_window, [](GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		auto platform = static_cast<Platform *>(glfwGetWindowUserPointer(window));
		for (auto &callback: platform->_key_callbacks)
			callback(key, scancode, action, mods);
	});
	glfwSetMouseButtonCallback(_current_window, [](GLFWwindow *window, int button, int action, int mods)
	{
		auto platform = static_cast<Platform *>(glfwGetWindowUserPointer(window));
		for (auto &callback: platform->_mouse_callbacks)
			callback(button, action, mods);
	});
	glfwSetScrollCallback(_current_window, [](GLFWwindow *window, double xoffset, double yoffset)
	{
		auto platform = static_cast<Platform *>(glfwGetWindowUserPointer(window));
		for (auto &callback: platform->_scroll_callbacks)
			callback(xoffset, yoffset);
	});
	glfwSetCursorPosCallback(_current_window, [](GLFWwindow *window, double xpos, double ypos)
	{
		auto platform = static_cast<Platform *>(glfwGetWindowUserPointer(window));
		for (auto &callback: platform->_cursor_callbacks)
			callback(xpos, ypos);
	});

	if (!_inited)
	{
		if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
			throw std::runtime_error("Failed to initialize GLAD");

		if (_opt.MSAA)
			glEnable(GL_MULTISAMPLE);

		ImGui::CreateContext();
		ImPlot::CreateContext();
		ImGui_ImplGlfw_InitForOpenGL(_current_window, true); // TODO: install callbacks?
		ImGui_ImplOpenGL3_Init();

		ImFontConfig config;
		config.FontDataOwnedByAtlas = false;
		ImGui::GetIO().IniFilename = nullptr;
		ImGui::GetIO().Fonts->Clear();
		ImGui::GetIO().Fonts->AddFontFromMemoryTTF(font_ttf, static_cast<int>(font_ttf_len), 14.0f, &config);
		ImGui::GetIO().Fonts->Build();

//		ImGui::StyleColorsLight();
//		ImPlot::StyleColorsLight();
		ImGui::StyleColorsDark();
		ImPlot::StyleColorsDark();
		_inited = true;
	}
}

void Kasumi::Platform::_rendering_loop(App &app)
{
	while (!glfwWindowShouldClose(_current_window) || app.quit())
	{
		_begin_frame();
		_color_picker();
		_benchmark();
		_menu(app);
		_monitor(app);
		_update(app);
		_end_frame();
	}
}

void Kasumi::Platform::_clear_window()
{
	if (_opt.clear_color)
	{
		glClearColor(_opt.background_color[0], _opt.background_color[1], _opt.background_color[2], 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	if (_opt.clear_depth)
	{
		glEnable(GL_DEPTH_TEST);
		glClear(GL_DEPTH_BUFFER_BIT);
	}
	if (_opt.clear_stencil)
		glClear(GL_STENCIL_BUFFER_BIT);
}

void Kasumi::Platform::_begin_frame()
{
	_clear_window();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void Kasumi::Platform::_end_frame()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(_current_window);
	glfwPollEvents();
}
void Kasumi::Platform::_menu(Kasumi::App &app)
{
	app.ui_menu();
}
void Kasumi::Platform::_benchmark() const
{
	if (!_opt.show_benchmark)
		return;

	Timer::t += ImGui::GetIO().DeltaTime;
	Timer::offset = (Timer::offset + 1) % Timer::max_size;

	ImGui::SetNextWindowPos({ImGui::GetIO().DisplaySize.x * 0.6f, ImGui::GetIO().DisplaySize.y * 0.0f}, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ImGui::GetIO().DisplaySize.x * 0.4f, ImGui::GetIO().DisplaySize.y * 0.2f}, ImGuiCond_FirstUseEver);
	ImGui::Begin("Benchmark", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	static float history = 10.0f;
	static ImPlotAxisFlags flags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_Opposite;
	if (ImPlot::BeginPlot("##Benchmark", ImVec2(-1, -1)))
	{
		ImPlot::SetupAxes(nullptr, nullptr, flags, flags);
		ImPlot::SetupAxisLimits(ImAxis_X1, Timer::t - history, Timer::t, ImGuiCond_Always);
		ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1);
		for (auto &bm: Timer::bench_marker)
		{
			auto &title = bm.first;
			auto &data = bm.second;
//			ImPlot::PlotShaded(title.c_str(), &data[0].x, &data[0].y, data.size(), (real) -INFINITY, 0, Timer::offset, 2 * sizeof(float));
			ImPlot::PlotLine(title.c_str(), &data[0].x, &data[0].y, data.size(), 0, Timer::offset, 2 * sizeof(float));
		}
		ImPlot::EndPlot();
	}
	ImGui::End();
}
void Kasumi::Platform::_monitor(App &app)
{
	ImGui::SetNextWindowPos({ImGui::GetIO().DisplaySize.x * 0.0f, ImGui::GetIO().DisplaySize.y * 0.2f}, ImGuiCond_FirstUseEver);
	ImGui::Begin("Monitor", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
	app.ui_sidebar();
	ImGui::End();
}
void Kasumi::Platform::_color_picker()
{
	if (!_opt.show_color_picker)
		return;

	ImGui::SetNextWindowPos({ImGui::GetIO().DisplaySize.x * 0.05f, ImGui::GetIO().DisplaySize.y * 0.1f}, ImGuiCond_FirstUseEver);
	ImGui::Begin("BackgroundColor", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize);
//	ImGui::PushItemWidth(100);
	ImGui::ColorPicker3("", _opt.background_color.data(), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoSidePreview);
//	ImGui::PopItemWidth();
	ImGui::End();
}
void Kasumi::Platform::_update(Kasumi::App &app)
{
	app.update(0.02);
}

Kasumi::App::App() : _platform(std::make_shared<Kasumi::Platform>(_opt.width, _opt.height))
{
	Shader::Init();
	Camera::Init();
	Camera::MainCamera->_opt.aspect_ratio = static_cast<float>(_opt.width) / static_cast<float>(_opt.height);
	Camera::MainCamera->_rebuild_();
	Light::Init();
}
void Kasumi::App::launch() { _platform->launch(*this); }
void Kasumi::App::key(int key, int scancode, int action, int mods)
{
}
auto Kasumi::App::quit() -> bool { return false; }
void Kasumi::App::mouse_button(int button, int action, int mods) {}
void Kasumi::App::mouse_scroll(double x_offset, double y_offset) {}
void Kasumi::App::mouse_cursor(double x_pos, double y_pos) {}
void Kasumi::App::ui_menu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open Scene (Ctrl+o)")) {}
			if (ImGui::MenuItem("Export Scene (Ctrl+e)")) {}
			if (ImGui::MenuItem("Save Scene (Ctrl+s)")) {}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo (Ctrl+z)")) {}
			if (ImGui::MenuItem("Redo (Ctrl+y)")) {}
			if (ImGui::MenuItem("Edit Debug Data (Ctrl+d)")) {}
			if (ImGui::MenuItem("Settings")) {}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Pathtracer"))
		{
			if (ImGui::MenuItem("Load Pathtracer")) {}
			ImGui::EndMenu();
		}

		ImGui::Text("FPS: %.0f", ImGui::GetIO().Framerate);
		ImGui::EndMainMenuBar();
	}
}
void Kasumi::App::ui_sidebar()
{
	if (_inspecting != nullptr)
		_inspecting->INSPECT();
}
