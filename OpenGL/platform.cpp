#include "glad/glad.h" // include glad before glfw
#include "GLFW/glfw3.h"
#include "../api.h"

#include "imgui.h"
#include "implot.h"
#include "imgui_internal.h"
#include "ImGuizmo.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"
#include "../font.dat"
#include "backends/platform.h"

#include <stdexcept>

#ifdef HinaImGuizmo
bool useWindow = true;
int gizmoCount = 1;
float camDistance = 8.f;
static const float identityMatrix[16] =
		{ 1.f, 0.f, 0.f, 0.f,
		  0.f, 1.f, 0.f, 0.f,
		  0.f, 0.f, 1.f, 0.f,
		  0.f, 0.f, 0.f, 1.f };
float objectMatrix[4][16] = {
		{ 1.f, 0.f, 0.f, 0.f,
				0.f, 1.f, 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				0.f, 0.f, 0.f, 1.f },

		{ 1.f, 0.f, 0.f, 0.f,
				0.f, 1.f, 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				2.f, 0.f, 0.f, 1.f },

		{ 1.f, 0.f, 0.f, 0.f,
				0.f, 1.f, 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				2.f, 0.f, 2.f, 1.f },

		{ 1.f, 0.f, 0.f, 0.f,
				0.f, 1.f, 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				0.f, 0.f, 2.f, 1.f }
};
static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
void EditTransform(float* cameraView, float* cameraProjection, float* matrix, bool editTransformDecomposition)
{
	static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
	static bool useSnap = false;
	static float snap[3] = { 1.f, 1.f, 1.f };
	static float bounds[] = { -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f };
	static float boundsSnap[] = { 0.1f, 0.1f, 0.1f };
	static bool boundSizing = false;
	static bool boundSizingSnap = false;

	if (editTransformDecomposition)
	{
		if (ImGui::IsKeyPressed(ImGuiKey_T))
			mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
		if (ImGui::IsKeyPressed(ImGuiKey_E))
			mCurrentGizmoOperation = ImGuizmo::ROTATE;
		if (ImGui::IsKeyPressed(ImGuiKey_R)) // r Key
			mCurrentGizmoOperation = ImGuizmo::SCALE;
		if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
			mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
			mCurrentGizmoOperation = ImGuizmo::ROTATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
			mCurrentGizmoOperation = ImGuizmo::SCALE;
		if (ImGui::RadioButton("Universal", mCurrentGizmoOperation == ImGuizmo::UNIVERSAL))
			mCurrentGizmoOperation = ImGuizmo::UNIVERSAL;
		float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		ImGuizmo::DecomposeMatrixToComponents(matrix, matrixTranslation, matrixRotation, matrixScale);
		ImGui::InputFloat3("Tr", matrixTranslation);
		ImGui::InputFloat3("Rt", matrixRotation);
		ImGui::InputFloat3("Sc", matrixScale);
		ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix);

		if (mCurrentGizmoOperation != ImGuizmo::SCALE)
		{
			if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
				mCurrentGizmoMode = ImGuizmo::LOCAL;
			ImGui::SameLine();
			if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
				mCurrentGizmoMode = ImGuizmo::WORLD;
		}
		if (ImGui::IsKeyPressed(ImGuiKey_S))
			useSnap = !useSnap;
		ImGui::Checkbox("##UseSnap", &useSnap);
		ImGui::SameLine();

		switch (mCurrentGizmoOperation)
		{
			case ImGuizmo::TRANSLATE:
				ImGui::InputFloat3("Snap", &snap[0]);
				break;
			case ImGuizmo::ROTATE:
				ImGui::InputFloat("Angle Snap", &snap[0]);
				break;
			case ImGuizmo::SCALE:
				ImGui::InputFloat("Scale Snap", &snap[0]);
				break;
		}
		ImGui::Checkbox("Bound Sizing", &boundSizing);
		if (boundSizing)
		{
			ImGui::PushID(3);
			ImGui::Checkbox("##BoundSizing", &boundSizingSnap);
			ImGui::SameLine();
			ImGui::InputFloat3("Snap", boundsSnap);
			ImGui::PopID();
		}
	}

	ImGuiIO& io = ImGui::GetIO();
	float viewManipulateRight = io.DisplaySize.x;
	float viewManipulateTop = 0;
	static ImGuiWindowFlags gizmoWindowFlags = 0;
	if (useWindow)
	{
		ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_Appearing);
		ImGui::SetNextWindowPos(ImVec2(400,20), ImGuiCond_Appearing);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, (ImVec4)ImColor(0.35f, 0.3f, 0.3f));
		ImGui::Begin("Gizmo", 0, gizmoWindowFlags);
		ImGuizmo::SetDrawlist();
		float windowWidth = (float)ImGui::GetWindowWidth();
		float windowHeight = (float)ImGui::GetWindowHeight();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
		viewManipulateRight = ImGui::GetWindowPos().x + windowWidth;
		viewManipulateTop = ImGui::GetWindowPos().y;
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		gizmoWindowFlags = ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(window->InnerRect.Min, window->InnerRect.Max) ? ImGuiWindowFlags_NoMove : 0;
	}
	else
	{
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
	}

	ImGuizmo::DrawGrid(cameraView, cameraProjection, identityMatrix, 100.f);
	ImGuizmo::DrawCubes(cameraView, cameraProjection, &objectMatrix[0][0], gizmoCount);
	ImGuizmo::Manipulate(cameraView, cameraProjection, mCurrentGizmoOperation, mCurrentGizmoMode, matrix, NULL, useSnap ? &snap[0] : NULL, boundSizing ? bounds : NULL, boundSizingSnap ? boundsSnap : NULL);

	ImGuizmo::ViewManipulate(cameraView, camDistance, ImVec2(viewManipulateRight - 128, viewManipulateTop), ImVec2(128, 128), 0x10101010);

	if (useWindow)
	{
		ImGui::End();
		ImGui::PopStyleColor(1);
	}
}
#endif

GLFWwindow *Kasumi::Platform::WINDOW = nullptr;
Kasumi::Platform::Platform(int width, int height, const std::string &title) : _inited(false), _width(width), _height(height), _current_window(nullptr)
{
	_new_window(_width, _height, title);
}
auto Kasumi::Platform::GetCursorPos() -> std::pair<double, double>
{
	if (Platform::WINDOW == nullptr)
		return {};
	double pos_x, pos_y;
	glfwGetCursorPos(Platform::WINDOW, &pos_x, &pos_y);
	return {pos_x, pos_y};
}
Kasumi::Platform::~Platform()
{
	glfwTerminate();
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
									if (key == GLFW_KEY_O && action == GLFW_PRESS)
										save_image("output.png");
								});
	_key_callbacks.emplace_back([&](int key, int scancode, int action, int mods)
								{
									Kasumi::Camera::MainCamera->key(key, scancode, action, mods);
									app.key(key, scancode, action, mods);
								});
	_mouse_callbacks.emplace_back([&](int button, int action, int mods)
								  {
									  Kasumi::Camera::MainCamera->mouse_button(button, action, mods);
									  app.mouse_button(button, action, mods);
								  });
	_scroll_callbacks.emplace_back([&](double x_offset, double y_offset)
								   {
									   Kasumi::Camera::MainCamera->mouse_scroll(x_offset, y_offset);
									   app.mouse_scroll(x_offset, y_offset);
								   });
	_cursor_callbacks.emplace_back([&](double x_pos, double y_pos)
								   {
									   Kasumi::Camera::MainCamera->mouse_cursor(x_pos, y_pos);
									   app.mouse_cursor(x_pos, y_pos);
								   });
	_rendering_loop(app);
}


#include "stb/stb_image_write.h"
void Kasumi::Platform::save_image(const std::string &filename) const
{
	int width, height;
	glfwGetFramebufferSize(_current_window, &width, &height);
	GLsizei nrChannels = 3;
	GLsizei stride = nrChannels * width;
	stride += (stride % 4) ? (4 - stride % 4) : 0;
	GLsizei bufferSize = stride * height;
	std::vector<char> buffer(bufferSize);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
	stbi_flip_vertically_on_write(true);
	stbi_write_png(filename.c_str(), width, height, nrChannels, buffer.data(), stride);
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
	if (_current_window != nullptr)
	{
		Platform::WINDOW = _current_window;
		_current_window_name = title;
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
	} else
	{
		_without_gui = true;
	}

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
	if (_without_gui)
	{
		while (!app.quit())
			_update(app);
	} else
	{
		while (!glfwWindowShouldClose(_current_window) && !app.quit())
		{
			_begin_frame();
			_color_picker();
			_benchmark();
			_menu(app);
			_monitor(app);
			_update(app);
			_end_frame();
			if (_opt.video_mode)
			{
				static int frame = 0;
				std::string frame_str = std::to_string(frame++);
				while (frame_str.size() < 6)
					frame_str = "0" + frame_str;
				save_image("frame_" + frame_str + ".png");
			}
		}
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

#ifdef HinaImGuizmo
	ImGuizmo::BeginFrame();
	static float cameraView[16] =
			{ 1.f, 0.f, 0.f, 0.f,
			  0.f, 1.f, 0.f, 0.f,
			  0.f, 0.f, 1.f, 0.f,
			  0.f, 0.f, 0.f, 1.f };

	static float cameraProjection[16];


	int matId = 1;
	ImGuizmo::SetID(matId);
	EditTransform(cameraView, cameraProjection, objectMatrix[matId], true);
#endif
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
	if (!_opt.show_menu)
		return;

	app.ui_menu();
}
void Kasumi::Platform::_benchmark() const
{
	if (!_opt.show_benchmark)
		return;

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

	Timer::t += ImGui::GetIO().DeltaTime;
	Timer::offset = (Timer::offset + 1) % Timer::max_size;

	ImGui::End();
}
void Kasumi::Platform::_monitor(App &app) const
{
	if (!_opt.show_inspector)
		return;

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
	GLint m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);
	app.update_viewport(m_viewport[2], m_viewport[3]);
}

Kasumi::App::App() : _platform(std::make_shared<Kasumi::Platform>(_opt.width, _opt.height))
{
	Shader::Init();
	Camera::Init();
	Camera::MainCamera->_opt.width = static_cast<float>(_opt.width);
	Camera::MainCamera->_opt.height = static_cast<float>(_opt.height);
	Camera::MainCamera->_opt.aspect_ratio = static_cast<float>(_opt.width) / static_cast<float>(_opt.height);
	Camera::MainCamera->_rebuild_();
	Light::Init();
}
void Kasumi::App::launch() { _platform->launch(*this); }
void Kasumi::App::inspect(Kasumi::INSPECTOR *ptr) { _inspectors.emplace_back(ptr); }
void Kasumi::App::resize(int width, int height)
{
	glfwSetWindowSize(_platform->_current_window, width, height);
	glViewport(0, 0, width, height);
	update_viewport(width, height);
}
void Kasumi::App::key(int key, int scancode, int action, int mods) {}
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
	for (auto &inspector: _inspectors)
		inspector->INSPECT();
}
void Kasumi::App::update_viewport(int width, int height)
{
	if (width == _opt.width && height == _opt.height)
		return;
#ifdef __APPLE__
	_opt.width = width / 2;
	_opt.height = height / 2;
#else
	_opt.width = width;
	_opt.height = height;
#endif
	Camera::MainCamera->_opt.width = static_cast<float>(_opt.width);
	Camera::MainCamera->_opt.height = static_cast<float>(_opt.height);
	Camera::MainCamera->_opt.aspect_ratio = static_cast<float>(_opt.width) / static_cast<float>(_opt.height);
	Camera::MainCamera->_rebuild_();
}
