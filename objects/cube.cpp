#include "cube.h"

Kasumi::CubeObject::CubeObject()
{
	NAME = "Cube";
	_shader = Shader::DefaultMeshShader;
	_init("cube", "");
	load_surface(this);
}
void Kasumi::CubeObject::_update_surface()
{
	if (_dirty)
	{
		_transform = mTransform3(POSE.position, mQuaternion(POSE.euler));
		_bound = mBBox3(-POSE.scale, POSE.scale);
		_extent = {POSE.scale.x(), POSE.scale.y(), POSE.scale.z()};
	}
}
