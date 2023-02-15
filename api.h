#ifndef KASUMI_API_H
#define KASUMI_API_H

// Copyright (c) 2023 Xayah Hina
// MPL-2.0 license

#include "platform.h"
#include "shader.h"
#include "camera.h"

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
};

class Renderable
{
public:
	Renderable() = default;
	virtual ~Renderable() = default;

	virtual void render() final
	{
		_shader->use();
		_update_uniform();
		_draw();
	}

	ShaderPtr _shader = Shader::DefaultMeshShader;

protected:
	virtual auto _get_model() -> mMatrix4x4 = 0;
	virtual auto _get_view() -> mMatrix4x4 final { return Camera::MainCamera->get_view(); }
	virtual auto _get_projection() -> mMatrix4x4 final { return Camera::MainCamera->get_projection(); }
	virtual void _update_uniform()
	{
		_shader->uniform("model", _get_model());
		_shader->uniform("view", _get_view());
		_shader->uniform("projection", _get_projection());
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

#endif //KASUMI_API_H
