#ifndef KASUMI_INSPECTOR_H
#define KASUMI_INSPECTOR_H

// Copyright (c) 2023 Xayah Hina
// MPL-2.0 license

#include "shader.h"
#include "imgui.h"

namespace Kasumi
{
class VALID_CHECKER
{
public:
	virtual auto VALID() const -> bool { return true; }
};

class Inspector
{
protected:
	friend class App;
	virtual void inspect() = 0;
};
using InspectorPtr = std::shared_ptr<Inspector>;

class Renderable : public VALID_CHECKER
{
public:
	virtual void render()
	{
		update_mvp();
	}
	virtual void update_mvp() final
	{
		_shader->use();
		_shader->uniform("model", _get_model());
		_shader->uniform("view", _get_view());
		_shader->uniform("projection", _get_projection());
	}

	auto VALID() const -> bool override
	{
		return
				VALID_CHECKER::VALID() &&
				_shader != nullptr &&
				_get_model != nullptr &&
				_get_view != nullptr &&
				_get_projection != nullptr;
	}

protected:
	ShaderPtr _shader = nullptr;
	std::function<mMatrix4x4()> _get_model = nullptr;
	std::function<mMatrix4x4()> _get_view = nullptr;
	std::function<mMatrix4x4()> _get_projection = nullptr;
};
using RenderablePtr = std::shared_ptr<Renderable>;
} // namespace Kasumi

#endif //KASUMI_INSPECTOR_H
