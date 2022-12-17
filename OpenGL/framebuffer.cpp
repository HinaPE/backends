#include "glad/glad.h"
#include "../framebuffer.h"

#include <array>
#include <stdexcept>

Kasumi::Framebuffer::Framebuffer(int width, int height, float base_x, float base_y, float top_x, float top_y)
		: _width(width), _height(height), _base_x(base_x), _base_y(base_y), _top_x(top_x), _top_y(top_y), _fbo(0), _vao(0), _texture(0), _screen_shader(std::make_shared<Kasumi::Shader>(std::string(BuiltinShaderDir) + "screen_vertex.glsl", std::string(BuiltinShaderDir) + "screen_fragment.glsl")) { setup(); }

void Kasumi::Framebuffer::render() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glClearColor(_opt.background_color.x, _opt.background_color.y, _opt.background_color.z, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	if (render_callback)
		render_callback();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	_screen_shader->use();
	_screen_shader->uniform("screenTexture", 0);
	glBindVertexArray(_vao);
	glBindTexture(GL_TEXTURE_2D, _texture);
	glActiveTexture(GL_TEXTURE0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

void Kasumi::Framebuffer::setup()
{
	std::array<float, 24> screen = {
			_base_x, _base_y, 0.0, 0.0,
			_base_x, _top_y, 0.0, 1.0,
			_top_x, _base_y, 1.0, 0.0,

			_top_x, _base_y, 1.0, 0.0,
			_base_x, _top_y, 0.0, 1.0,
			_top_x, _top_y, 1.0, 1.0
	};
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), &screen[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void *>(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void *>(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);


	glGenFramebuffers(1, &_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

	glGenTextures(1, &_texture);
	glBindTexture(GL_TEXTURE_2D, _texture);
#ifdef __APPLE__
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2 * _width, 2 * _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
#else
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
#endif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture, 0);

	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
#ifdef __APPLE__
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 2 * _width, 2 * _height);
#else
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, _width, _height);
#endif
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw std::runtime_error("Framebuffer is not complete!");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
Kasumi::Framebuffer::~Framebuffer() { glDeleteFramebuffers(1, &_fbo); }
