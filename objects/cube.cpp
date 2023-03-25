#include "cube.h"

Kasumi::CubeObject::CubeObject()
{
	NAME = "Cube";
	_shader = Shader::DefaultMeshShader;
	_init("cube", "");
	load_surface(this);

	_transform = mTransform3(POSE.position, mQuaternion(POSE.euler));
	_bound = mBBox3(-POSE.scale, POSE.scale);
	_extent = {POSE.scale.x(), POSE.scale.y(), POSE.scale.z()};
}

void add_wall(const mVector3 &minX, const mVector3 &maxX, real radius, std::vector<mVector3> *target_boundary)
{
	const real diam = 1.4 * radius;
	const int stepsX = (int) ((maxX.x() - minX.x()) / diam) + 1;
	const int stepsY = (int) ((maxX.y() - minX.y()) / diam) + 1;
	const int stepsZ = (int) ((maxX.z() - minX.z()) / diam) + 1;

	for (int i = 0; i < stepsX; ++i)
	{
		for (int j = 0; j < stepsY; ++j)
		{
			for (int k = 0; k < stepsZ; ++k)
			{
				const real x = minX.x() + i * diam;
				const real y = minX.y() + j * diam;
				const real z = minX.z() + k * diam;
				target_boundary->emplace_back(x, y, z);
			}
		}
	}
}

auto Kasumi::CubeObject::generate_surface() const -> std::vector<mVector3>
{
	std::vector<mVector3> surface_points;
	surface_points.clear();

	const auto half_width = _extent.x();
	const auto half_height = _extent.y();
	const auto half_depth = _extent.z();

	const real x1 = POSE.position.x() - half_width;
	const real x2 = POSE.position.x() + half_width;
	const real y1 = POSE.position.y() - half_height;
	const real y2 = POSE.position.y() + half_height;
	const real z1 = POSE.position.z() - half_depth;
	const real z2 = POSE.position.z() + half_depth;

	add_wall(mVector3(x1, y1, z1), mVector3(x2, y1, z2), 0.029, &surface_points); // floor
	add_wall(mVector3(x1, y2, z1), mVector3(x2, y2, z2), 0.029, &surface_points); // top
	add_wall(mVector3(x1, y1, z1), mVector3(x1, y2, z2), 0.029, &surface_points); // left
	add_wall(mVector3(x2, y1, z1), mVector3(x2, y2, z2), 0.029, &surface_points); // right
	add_wall(mVector3(x1, y1, z1), mVector3(x2, y2, z1), 0.029, &surface_points); // back
	add_wall(mVector3(x1, y1, z2), mVector3(x2, y2, z2), 0.029, &surface_points); // front

	return surface_points;
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
