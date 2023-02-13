#include "../light.h"

Kasumi::Light::Light(Kasumi::Light::Opt opt) : _opt(std::move(opt)) {}
Kasumi::Light::~Light()
= default;

void Kasumi::Light::update(const Kasumi::CameraPtr &camera) { _opt.view_pos = camera->_opt.position; }
void Kasumi::Light::sync_shader(const Kasumi::Shader &shader) const
{
	shader.use();
	shader.uniform("lightPos", _opt.light_pos);
	shader.uniform("viewPos", _opt.view_pos);
}
void Kasumi::Light::render()
{
	// TODO: support render a light object
}
