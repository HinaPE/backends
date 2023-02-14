#ifndef KASUMI_FRAMEBUFFER_H
#define KASUMI_FRAMEBUFFER_H

// Copyright (c) 2023 Xayah Hina
// MPL-2.0 license

// Dependency:
// - Math Backend
// - Shader

#include "shader.h"

#include <functional>
#include <memory>

namespace Kasumi
{
class Framebuffer
{
public:
	void render() const;
	std::function<void()> render_callback = nullptr;

public:
	struct Opt
	{
		mVector3 background_color = {0.9, 0.9, 0.9};
	} _opt;

public:
	Framebuffer(int width, int height, float base_x = -1, float base_y = -1, float top_x = 1, float top_y = 1);
	~Framebuffer();

private:
	void setup();

private:
	unsigned int _fbo, _vao;
	unsigned int _texture;
	int _width, _height;
	float _base_x, _base_y;
	float _top_x = 1.0, _top_y = 1.0;
};
using FramebufferPtr = std::shared_ptr<Framebuffer>;
}

#endif //KASUMI_FRAMEBUFFER_H
