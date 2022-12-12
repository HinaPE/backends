#ifndef KASUMI_FRAMEBUFFER_H
#define KASUMI_FRAMEBUFFER_H

#include <memory>

namespace Kasumi
{
class Framebuffer
{
public:
	void use() const;
	void unuse() const;
	void bind_texture() const;

public:
	Framebuffer(int width, int height);
	~Framebuffer();

private:
	void setup();

private:
	unsigned int _fbo;
	unsigned int _texture;
	int _width, _height;
};
using FramebufferPtr = std::shared_ptr<Framebuffer>;
}

#endif //KASUMI_FRAMEBUFFER_H
