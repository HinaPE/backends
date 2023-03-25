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
auto Kasumi::SphereObject::generate_surface() const -> std::vector<mVector3>
{
	// generate sphere surface points
	std::vector<mVector3> surface_points;
	surface_points.clear();

	real theta = 0;
	real phi = 0;

	for (int i = 0; i < 100; ++i)
	{
		theta = 2 * HinaPE::Constant::PI * i / 100;
		for (int j = 0; j < 100; ++j)
		{
			phi = HinaPE::Constant::PI * j / 100;
			mVector3 point = _center + _radius * mVector3(sin(phi) * cos(theta), sin(phi) * sin(theta), cos(phi));
			surface_points.push_back(point);
		}
	}
	return surface_points;
}
