#ifndef BACKENDS_API_H
#define BACKENDS_API_H

// Copyright (c) 2023 Xayah Hina
// MPL-2.0 license

#include "platform.h"
#include "shader.h"
#include "texture.h"
#include "camera.h"
#include "light.h"
#include "mesh.h"
#include "model.h"
#include "pose.h"
#include "framebuffer.h"
#include "timer.h"

#include "imgui.h"

namespace Kasumi
{
class VALID_CHECKER
{
protected:
	friend class Scene2D;
	friend class Scene3D;
	virtual void VALID_CHECK() const {}
};

class INSPECTOR
{
protected:
	friend class App;
	virtual void INSPECT() = 0;

protected:
	virtual void INSPECT_BOOL(bool &data, const std::string &str) final { ImGui::Checkbox(str.c_str(), &data); }
	virtual void INSPECT_INT(int &data, const std::string &str) final { ImGui::DragScalar(str.c_str(), ImGuiDataType_S32, &data, 0.1, &HinaPE::Constant::I_INT_MIN, &HinaPE::Constant::I_INT_MAX, "%d"); }
	virtual void INSPECT_SIZE(size_t &data, const std::string &str) final { ImGui::DragScalar(str.c_str(), ImGuiDataType_U32, &data, 0.1, &HinaPE::Constant::I_SIZE_MIN, &HinaPE::Constant::I_SIZE_MAX, "%u"); }
	virtual void INSPECT_REAL(real &data, const std::string &str) final { ImGui::DragScalar(str.c_str(), ImGuiDataType_Real, &data, 0.1, &HinaPE::Constant::I_REAL_MIN, &HinaPE::Constant::I_REAL_MAX, "%.2f"); }
	virtual void INSPECT_VEC3(mVector3 &data, const std::string &str) final { ImGui::DragScalarN(str.c_str(), ImGuiDataType_Real, &data, 3, 0.1, &HinaPE::Constant::I_REAL_MIN, &HinaPE::Constant::I_REAL_MAX, "%.2f"); }
};

class IDBase
{
public:
	IDBase() : ID(ID_GLOBAL++) {}

	static unsigned int ID_GLOBAL;
	const unsigned int ID;
};

class NameBase
{
public:
	std::string NAME = "Untitled";
};

class UPDATE_PER_FRAME
{
protected:
	friend class App;
	virtual void UPDATE() = 0;
};

class PoseBase : public INSPECTOR, public UPDATE_PER_FRAME
{
public:
	Pose POSE;
	void track(mVector3 *pos, mVector3 *euler = nullptr, mVector3 *scale = nullptr)
	{
		track_pos = pos;
		track_euler = euler;
		track_scale = scale;
	}
	explicit PoseBase(mVector3 position = {0, 0, 0}, mVector3 euler = {0, 0, 0}, mVector3 scale = {1, 1, 1}) : POSE(std::move(position), std::move(euler), std::move(scale)) {}

protected:
	void INSPECT() override
	{
		ImGui::Text("Transform");
		auto sliders = [&](const std::string &label, mVector3 &data, float sens)
		{
			if (ImGui::DragScalarN(label.c_str(), ImGuiDataType_Real, &data[0], 3, sens, &HinaPE::Constant::I_REAL_MIN, &HinaPE::Constant::I_REAL_MAX, "%.2f"))
				_dirty = true;
		};
		sliders("Position", POSE.position, 0.1f);
		sliders("Rotation", POSE.euler, 0.1f);
		sliders("Scale", POSE.scale, 0.031f);
	}

	void UPDATE() override
	{
		if (track_pos != nullptr)
			if (!HinaPE::Math::similar(*track_pos, POSE.position))
				POSE.position = *track_pos;

		if (track_euler != nullptr)
			if (!HinaPE::Math::similar(*track_euler, POSE.euler))
				POSE.euler = *track_euler;

		if (track_scale != nullptr)
			if (!HinaPE::Math::similar(*track_scale, POSE.scale))
				POSE.scale = *track_scale;
	}

	bool _dirty = true;

private:
	mVector3 *track_pos = nullptr;
	mVector3 *track_euler = nullptr;
	mVector3 *track_scale = nullptr;
};

class InstancePosesBase : public PoseBase
{
public:
	std::vector<Pose> POSES;
	mVector3 DEFAULT_POSITION = {0, 0, 0};
	mVector3 DEFAULT_EULER = {0, 0, 0};
	mVector3 DEFAULT_SCALE = {1, 1, 1};
	void track(std::vector<mVector3> *pos, std::vector<mVector3> *euler = nullptr, std::vector<mVector3> *scale = nullptr)
	{
		track_poss = pos;
		track_eulers = euler;
		track_scales = scale;
	}
	explicit InstancePosesBase() { POSES.clear(); }

protected:
	friend class Scene3D;
	void UPDATE() override
	{
		PoseBase::UPDATE();

		if (track_poss == nullptr)
			return;

		if (!_poses_dirty)
			return;

		auto size = (*track_poss).size();
		POSES.resize(size);
		for (int i = 0; i < size; ++i)
		{
			POSES[i].position = (*track_poss)[i];
			if (track_eulers != nullptr)
				POSES[i].euler = (*track_eulers)[i];
			else
				POSES[i].euler = DEFAULT_EULER;
			if (track_scales != nullptr)
				POSES[i].scale = (*track_scales)[i];
			else
				POSES[i].scale = DEFAULT_SCALE;
		}
//		_poses_dirty = false; // TODO: temporarily disable this
	}

	bool _poses_dirty = true;

private:
	std::vector<mVector3> *track_poss = nullptr;
	std::vector<mVector3> *track_eulers = nullptr;
	std::vector<mVector3> *track_scales = nullptr;
};

class Renderable
{
public:
	Renderable() = default;
	virtual ~Renderable() = default;

	virtual void render() final
	{
		_update_uniform();
		_draw();
	}
	virtual void render_volume() final
	{
		_update_uniform();
		_draw_volume();
	}

	ShaderPtr _shader = nullptr;

protected:
	virtual void _update_uniform()
	{
		_shader->use();
		_shader->uniform("view", Camera::MainCamera->get_view());
		_shader->uniform("projection", Camera::MainCamera->get_projection());
		_shader->uniform("lightPos", Light::MainLight->_opt.light_pos);
		_shader->uniform("viewPos", Light::MainLight->_opt.view_pos);

		Shader::DefaultLineShader->use();
		Shader::DefaultLineShader->uniform("view", Camera::MainCamera->get_view());
		Shader::DefaultLineShader->uniform("projection", Camera::MainCamera->get_projection());
		Shader::DefaultLineShader->uniform("lightPos", Light::MainLight->_opt.light_pos);
		Shader::DefaultLineShader->uniform("viewPos", Light::MainLight->_opt.view_pos);
	}
	virtual void _draw() = 0;
	virtual void _draw_volume() {}
};

class App
{
public:
	virtual void launch() final;
	virtual void inspect(INSPECTOR *ptr) final;

	// options
	void clean_mode()
	{
		close_inspector();
		close_benchmark();
	}
	void light_mode() { _platform->_opt.background_color = {1, 1, 1}; }
	void dark_mode() { _platform->_opt.background_color = {0.1, 0.1, 0.1}; }
	void close_inspector() { _platform->_opt.show_inspector = false; }
	void close_benchmark() { _platform->_opt.show_benchmark = false; }

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
#ifdef __APPLE__
		int width = 1500, height = 900;
#else
		int width = 1024, height = 768;
#endif
	} _opt;
	App();

protected:
	friend class Platform;
	PlatformPtr _platform;
	std::vector<INSPECTOR *> _inspectors;
};

template<class SrcType>
auto is_renderable(const SrcType *src) -> bool { return dynamic_cast<const Renderable *>(src) != nullptr; }
template<class SrcType>
auto need_valid_check(const SrcType *src) -> bool { return dynamic_cast<const VALID_CHECKER *>(src) != nullptr; }
template<class SrcType>
auto as_renderable(SrcType *src) -> Renderable * { return dynamic_cast<Renderable *>(src); }
template<class SrcType>
auto as_valid_check(SrcType *src) -> VALID_CHECKER * { return dynamic_cast<VALID_CHECKER *>(src); }

using INSPECTORPtr = std::shared_ptr<INSPECTOR>;
using RenderablePtr = std::shared_ptr<Renderable>;


// ======================== 2D ========================
class Pose2DBase : public INSPECTOR, public UPDATE_PER_FRAME
{
public:
	Pose2D POSE;
	bool _dirty = true;
	void track(mVector2 *pos, float *euler = nullptr, mVector2 *scale = nullptr)
	{
		track_pos = pos;
		track_euler = euler;
		track_scale = scale;
	}

protected:
	void INSPECT() override
	{
		ImGui::Text("Transform");
		ImGui::DragScalarN("Position", ImGuiDataType_Real, &POSE.position[0], 2, 0.1f, &HinaPE::Constant::I_REAL_MIN, &HinaPE::Constant::I_REAL_MAX, "%.2f");
		ImGui::DragScalarN("Rotation", ImGuiDataType_Real, &POSE.rotation, 1, 0.1f, &HinaPE::Constant::I_REAL_MIN, &HinaPE::Constant::I_REAL_MAX, "%.2f");
		ImGui::DragScalarN("Scale", ImGuiDataType_Real, &POSE.scale[0], 2, 0.031f, &HinaPE::Constant::I_REAL_MIN, &HinaPE::Constant::I_REAL_MAX, "%.2f");
	}

	void UPDATE() override
	{
		if (track_pos != nullptr)
			if (!HinaPE::Math::similar(*track_pos, POSE.position))
				POSE.position = *track_pos;

		if (track_euler != nullptr)
			if (!HinaPE::Math::similar(*track_euler, POSE.rotation))
				POSE.rotation = *track_euler;

		if (track_scale != nullptr)
			if (!HinaPE::Math::similar(*track_scale, POSE.scale))
				POSE.scale = *track_scale;
	}

private:
	mVector2 *track_pos = nullptr;
	float *track_euler = nullptr;
	mVector2 *track_scale = nullptr;
};
} // namespace Kasumi

#endif //BACKENDS_API_H