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
#include "pose.h"
#include "framebuffer.h"

#include "imgui.h"

namespace Kasumi
{
class VALID_CHECKER
{
public:
	virtual void VALID_CHECK() const {}
};

class INSPECTOR
{
public:
	friend class App;
	virtual void INSPECT() = 0;

protected:
	virtual void INSPECT_REAL(real &data, const std::string &str) final { ImGui::DragScalar(str.c_str(), ImGuiDataType_Real, &data, 0.1, &REAL_MIN, &REAL_MAX, "%.2f"); }
	virtual void INSPECT_VEC3(mVector3 &data, const std::string &str) final { ImGui::DragScalarN(str.c_str(), ImGuiDataType_Real, &data, 3, 0.1, &REAL_MIN, &REAL_MAX, "%.2f"); }

protected:
	real REAL_MIN = -std::numeric_limits<real>::max();
	real REAL_MAX = std::numeric_limits<real>::max();
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
} // namespace Kasumi

#endif //BACKENDS_API_H
