#include "glad/glad.h" // include glad before glfw
#include "GLFW/glfw3.h"
#include "../api.h"

#include "imgui.h"
#include "implot.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"
#include "../font.dat"

#include <stdexcept>

static float next_x = 0.f, next_y = 0.f;

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
		_inited = true;
	}
}

void Kasumi::Platform::_rendering_loop(App &app)
{
	while (!glfwWindowShouldClose(_current_window) || app.quit())
	{
		_begin_frame();
		app.ui_menu();
		ImGui::SetNextWindowPos({ImGui::GetIO().DisplaySize.x * 0.8f, ImGui::GetIO().DisplaySize.y * 0.2f}, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize({ImGui::GetIO().DisplaySize.x * 0.2f, ImGui::GetIO().DisplaySize.y * 0.0f}, ImGuiCond_FirstUseEver);
		ImGui::Begin("Monitor", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing);
		app.ui_sidebar();
		ImGui::End();
		app.update(0.02);
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
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		_opt.running = !_opt.running;
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
		ImGui::Text("Physics Update: %.0f ms", (_last_update_time * 1000));
		next_x = 0.f;
		next_y = ImGui::GetWindowSize().y;
		ImGui::EndMainMenuBar();
	}
}
// utility structure for realtime plot
struct ScrollingBuffer {
	int MaxSize;
	int Offset;
	ImVector<ImVec2> Data;
	ScrollingBuffer(int max_size = 2000) {
		MaxSize = max_size;
		Offset  = 0;
		Data.reserve(MaxSize);
	}
	void AddPoint(float x, float y) {
		if (Data.size() < MaxSize)
			Data.push_back(ImVec2(x,y));
		else {
			Data[Offset] = ImVec2(x,y);
			Offset =  (Offset + 1) % MaxSize;
		}
	}
	void Erase() {
		if (Data.size() > 0) {
			Data.shrink(0);
			Offset  = 0;
		}
	}
};
// utility structure for realtime plot
struct RollingBuffer {
	float Span;
	ImVector<ImVec2> Data;
	RollingBuffer() {
		Span = 10.0f;
		Data.reserve(2000);
	}
	void AddPoint(float x, float y) {
		float xmod = fmodf(x, Span);
		if (!Data.empty() && xmod < Data.back().x)
			Data.shrink(0);
		Data.push_back(ImVec2(xmod, y));
	}
};
void Kasumi::App::ui_sidebar()
{
	if (_inspecting != nullptr)
		_inspecting->INSPECT();

	ImGui::BulletText("Move your mouse to change the data!");
	ImGui::BulletText("This example assumes 60 FPS. Higher FPS requires larger buffer size.");
	static ScrollingBuffer sdata1, sdata2;
	static RollingBuffer   rdata1, rdata2;
	ImVec2 mouse = ImGui::GetMousePos();
	static float t = 0;
	t += ImGui::GetIO().DeltaTime;
	sdata1.AddPoint(t, mouse.x * 0.0005f);
	rdata1.AddPoint(t, mouse.x * 0.0005f);
	sdata2.AddPoint(t, mouse.y * 0.0005f);
	rdata2.AddPoint(t, mouse.y * 0.0005f);

	static float history = 10.0f;
	ImGui::SliderFloat("History",&history,1,30,"%.1f s");
	rdata1.Span = history;
	rdata2.Span = history;

	static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels;

	if (ImPlot::BeginPlot("##Scrolling", ImVec2(-1,150))) {
		ImPlot::SetupAxes(NULL, NULL, flags, flags);
		ImPlot::SetupAxisLimits(ImAxis_X1,t - history, t, ImGuiCond_Always);
		ImPlot::SetupAxisLimits(ImAxis_Y1,0,1);
		ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL,0.5f);
		ImPlot::PlotShaded("Mouse X", &sdata1.Data[0].x, &sdata1.Data[0].y, sdata1.Data.size(), -INFINITY, 0, sdata1.Offset, 2 * sizeof(float));
		ImPlot::PlotLine("Mouse Y", &sdata2.Data[0].x, &sdata2.Data[0].y, sdata2.Data.size(), 0, sdata2.Offset, 2*sizeof(float));
		ImPlot::EndPlot();
	}
	if (ImPlot::BeginPlot("##Rolling", ImVec2(-1,150))) {
		ImPlot::SetupAxes(NULL, NULL, flags, flags);
		ImPlot::SetupAxisLimits(ImAxis_X1,0,history, ImGuiCond_Always);
		ImPlot::SetupAxisLimits(ImAxis_Y1,0,1);
		ImPlot::PlotLine("Mouse X", &rdata1.Data[0].x, &rdata1.Data[0].y, rdata1.Data.size(), 0, 0, 2 * sizeof(float));
		ImPlot::PlotLine("Mouse Y", &rdata2.Data[0].x, &rdata2.Data[0].y, rdata2.Data.size(), 0, 0, 2 * sizeof(float));
		ImPlot::EndPlot();
	}

	ImGui::ColorPicker3("Background", _platform->_opt.background_color.data(), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel);
	next_x += ImGui::GetWindowSize().x;
}
