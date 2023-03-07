#include "arrow.h"

Kasumi::ArrowObject::ArrowObject()
{
	NAME = "Arrow";
	_shader = Shader::DefaultMeshShader;
	_init("arrow", "");
}
void Kasumi::ArrowObject::_update_surface()
{
	if (!_dirty)
	{
		_origin = POSE.position;
		_direction = mQuaternion(POSE.euler) * mVector3::UnitY();
		_dirty = false;
	}
}
