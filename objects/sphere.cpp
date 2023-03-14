#include "sphere.h"

Kasumi::SphereObject::SphereObject()
{
	NAME = "Sphere";
	_shader = Shader::DefaultMeshShader;
	_init("sphere", "");
	load_surface(this);
}
void Kasumi::SphereObject::_update_surface()
{
	if (_dirty)
	{
		_transform = mTransform3(POSE.position, mQuaternion(POSE.euler));
		_center = POSE.position;
		_radius = std::max(std::max(POSE.scale.x(), POSE.scale.y()), POSE.scale.z());
	}
}
