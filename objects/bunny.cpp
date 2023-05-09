#include "bunny.h"

Kasumi::BunnyObject::BunnyObject()
{
	NAME = "Bunny";
	_shader = Shader::DefaultMeshShader;
	_init("bunny", "");
	_mesh->centralize();
	load_surface(this);

	std::vector<mVector3> vertices;
	std::vector<unsigned int> indices;

	for (auto &v : _mesh->vertices())
		vertices.push_back(v.position);
	for (auto &i : _mesh->indices())
		indices.push_back(i);
	reload(vertices, indices);

	_transform = mTransform3(POSE.position, mQuaternion(POSE.euler));
}
auto Kasumi::BunnyObject::generate_surface() const -> std::vector<mVector3>
{
	return Collider3::generate_surface();
}
void Kasumi::BunnyObject::_update_surface()
{
	ObjectMesh3D::_update_surface();
}
