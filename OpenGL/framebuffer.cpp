#include "glad/glad.h"
#include "../framebuffer.h"

#include <iostream>

Kasumi::Framebuffer::Framebuffer(int width, int height) : _width(width), _height(height), _fbo(0) {}

void Kasumi::Framebuffer::use() const { glBindFramebuffer(GL_FRAMEBUFFER, _fbo); }
void Kasumi::Framebuffer::unuse() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
void Kasumi::Framebuffer::bind_texture() const { glBindTexture(GL_TEXTURE_2D, _texture); }

void Kasumi::Framebuffer::setup()
{
	glGenFramebuffers(1, &_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

	glGenTextures(1, &_texture);
	glBindTexture(GL_TEXTURE_2D, _texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _width, _height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture, 0);

	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer incomplete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
Kasumi::Framebuffer::~Framebuffer()
{
	glDeleteFramebuffers(1, &_fbo);
}
