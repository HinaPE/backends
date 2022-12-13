#ifndef KASUMI_FRAMEBUFFER_H
#define KASUMI_FRAMEBUFFER_H

#include "shader.h"

#include <functional>
#include <memory>

namespace Kasumi
{
class Framebuffer
{
public:
	void use() const;
	static void unuse() ;
	void render() const;
	std::function<void()> render_callback;

public:
	Framebuffer(int width, int height, float base_x, float base_y, float top_x, float top_y);
	~Framebuffer();

private:
	void setup();

private:
	Kasumi::ShaderPtr _screen_shader;
	unsigned int _fbo, _vao;
	unsigned int _texture;
	int _width, _height;
	float _base_x, _base_y;
	float _top_x = 1.0, _top_y = 1.0;
};
using FramebufferPtr = std::shared_ptr<Framebuffer>;
}

#endif //KASUMI_FRAMEBUFFER_H
