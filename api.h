#ifndef KASUMI_INSPECTOR_H
#define KASUMI_INSPECTOR_H

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
	virtual void VALID() const {}
};

class Inspector
{
protected:
	friend class App;
	virtual void _inspect() = 0;
};

class Renderable : public VALID_CHECKER
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

	ShaderPtr _shader = nullptr;

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

protected:
	// check valid
	void VALID() const override
	{
		if (_shader != nullptr)
			throw std::runtime_error("Shader is not set.");
	}
};

template<class SrcType>
auto is_renderable(const SrcType *src) -> bool { return dynamic_cast<const Renderable *>(src) != nullptr; }
template<class SrcType>
auto as_renderable(SrcType *src) -> Renderable * { return dynamic_cast<Renderable *>(src); }

using InspectorPtr = std::shared_ptr<Inspector>;
using RenderablePtr = std::shared_ptr<Renderable>;
} // namespace Kasumi

#endif //KASUMI_INSPECTOR_H
