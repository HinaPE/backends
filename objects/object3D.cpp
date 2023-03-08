#include "object3D.h"
#include "igl/ray_mesh_intersect.h"

unsigned int Kasumi::IDBase::ID_GLOBAL = 0;
std::shared_ptr<Kasumi::ObjectLines3D> Kasumi::ObjectLines3D::DefaultLines = nullptr;
std::shared_ptr<Kasumi::ObjectPoints3D> Kasumi::ObjectPoints3D::DefaultPoints = nullptr;

// ==================== Object3D ====================
auto Kasumi::ObjectMesh3D::ray_cast(const mRay3 &ray) const -> HinaPE::Geom::SurfaceRayIntersection3
{
	HinaPE::Geom::SurfaceRayIntersection3 res;
	const auto &verts_local = _mesh->_verts_eigen4;
	const auto &idxs = _mesh->_idxs_eigen;
	auto model = POSE.get_model_matrix()._m;
	auto t = (model * verts_local.transpose());
	Eigen::MatrixXd verts_world = t.transpose();

	Eigen::MatrixXd verts_world_3 = verts_world.block(0, 0, verts_world.rows(), 3);

	std::vector<igl::Hit> hits;
	res.is_intersecting = igl::ray_mesh_intersect(ray._origin._v, ray._direction._v, verts_world_3, idxs, hits);
	for (auto const &hit: hits)
	{
		res.point = ray._origin + static_cast<real>(hit.t) * ray._direction;
		res.distance = (static_cast<real>(hit.t) * ray._direction).length();
		res.ID = this->ID;
		return res;
	}
	return res;
}
void Kasumi::ObjectMesh3D::_draw()
{
	if (_mesh == nullptr) return;
	_update_surface();
	_mesh->render(*_shader);
}
void Kasumi::ObjectMesh3D::_update_uniform()
{
	Renderable::_update_uniform();
	_shader->uniform("model", POSE.get_model_matrix());
	Shader::DefaultLineShader->uniform("model", POSE.get_model_matrix());
}
void Kasumi::ObjectMesh3D::_init(const std::string &MESH, const std::string &TEXTURE, const mVector3 &COLOR) { _mesh = TEXTURE.empty() ? std::make_shared<Mesh>(MESH, COLOR) : _mesh = std::make_shared<Mesh>(MESH, TEXTURE); }
void Kasumi::ObjectMesh3D::_init(std::vector<Mesh::Vertex> &&vertices, std::vector<Mesh::Index> &&indices, std::map<std::string, std::vector<TexturePtr>> &&textures) { _mesh = std::make_shared<Mesh>(std::move(vertices), std::move(indices), std::move(textures)); }
void Kasumi::ObjectMesh3D::INSPECT()
{
	PoseBase::INSPECT();

	if (_mesh == nullptr)
		return;

	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Mesh Vertices: %zu", _mesh->vertices().size());
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Mesh Indices: %zu", _mesh->indices().size());
}
void Kasumi::ObjectMesh3D::VALID_CHECK() const
{
	if (_mesh == nullptr) throw std::runtime_error("Mesh is nullptr");
//	if (_surface == nullptr) throw std::runtime_error("Surface is nullptr");
}

// ==================== ObjectLines3D ====================
Kasumi::ObjectLines3D::ObjectLines3D()
{
	NAME = "Line";
	_shader = Shader::DefaultLineShader;
	_lines = std::make_shared<Lines>();
}
void Kasumi::ObjectLines3D::Init() { DefaultLines = std::make_shared<Kasumi::ObjectLines3D>(); }
void Kasumi::ObjectLines3D::add(const mVector3 &start, const mVector3 &end, const mVector3 &color) { _lines->add(start, end, color); }
void Kasumi::ObjectLines3D::clear() { _lines->clear(); }
void Kasumi::ObjectLines3D::_draw()
{
	if (_lines == nullptr) return;
	if (_lines->lines().empty()) return;
	_lines->render(*_shader);
}
void Kasumi::ObjectLines3D::_update_uniform()
{
	Renderable::_update_uniform();
	_shader->uniform("model", POSE.get_model_matrix());
}

// ==================== ObjectLines3DInstanced ====================
Kasumi::ObjectLines3DInstanced::ObjectLines3DInstanced()
{
	NAME = "Lines";
	_shader = Shader::DefaultInstanceLineShader;
	_init();
}
void Kasumi::ObjectLines3DInstanced::_init()
{
	auto init_lines = std::make_shared<Lines>();
	init_lines->add(mVector3(0, 0, 0), mVector3(1, 0, 0), HinaPE::Color::MAGENTA);
	_lines = std::make_shared<InstancedLines>(init_lines);
}
void Kasumi::ObjectLines3DInstanced::add(const mVector3 &start, const mVector3 &end, const mVector3 &color)
{
	mQuaternion q(mVector3::UnitX(), end - start);
	Pose pose;
	pose.position = start;
	pose.euler = q.euler();
	pose.scale.x() = (end - start).length();
	_poses.push_back(pose);
	_lines->_opt.instance_matrices.push_back(pose.get_model_matrix());
	_lines->_opt.dirty = true;
}
void Kasumi::ObjectLines3DInstanced::clear()
{
	_poses.clear();
	_lines->_opt.instance_matrices.clear();
	_dirty = true;
	_lines->_opt.dirty = true;
}
void Kasumi::ObjectLines3DInstanced::_draw()
{
	if (_lines == nullptr) return;
	_lines->render(*_shader);
}

// ==================== ObjectPoints3D ====================
Kasumi::ObjectPoints3D::ObjectPoints3D()
{
	NAME = "Points";
	_shader = Shader::DefaultPointShader;
	_points = std::make_shared<Points>();
}
void Kasumi::ObjectPoints3D::Init() { DefaultPoints = std::make_shared<Kasumi::ObjectPoints3D>(); }
void Kasumi::ObjectPoints3D::add(const mVector3 &point, const mVector3 &color) { _points->add(point, color); }
void Kasumi::ObjectPoints3D::clear() { _points->clear(); }
void Kasumi::ObjectPoints3D::_draw()
{
	if (_points == nullptr) return;
	_points->render(*_shader);
}
void Kasumi::ObjectPoints3D::_update_uniform()
{
	Renderable::_update_uniform();
	_shader->uniform("model", POSE.get_model_matrix());
}

// ==================== ObjectParticles3D ====================
Kasumi::ObjectParticles3D::ObjectParticles3D()
{
	NAME = "Particles";
	_shader = Shader::DefaultInstanceShader;
	_inst_id = -1;
	_init("cube", "");
}
void Kasumi::ObjectParticles3D::_init(const std::string &MESH, const std::string &TEXTURE, const mVector3 &COLOR)
{
	_mesh = TEXTURE.empty() ? std::make_shared<InstancedMesh>(std::make_shared<Mesh>(MESH, COLOR)) : std::make_shared<InstancedMesh>(std::make_shared<Mesh>(MESH, TEXTURE));
}
void Kasumi::ObjectParticles3D::_draw()
{
	if (_mesh == nullptr) return;
	_update_poses();
	if (_dirty) _update();
	_mesh->render(*_shader);
}
void Kasumi::ObjectParticles3D::_update()
{
	_mesh->_opt.instance_matrices.clear();
	_mesh->_opt.instance_matrices.reserve(_poses.size());

	for (auto &pose: _poses)
		_mesh->_opt.instance_matrices.push_back(pose.get_model_matrix());
	_mesh->_opt.dirty = true;
	_dirty = false;
}
void Kasumi::ObjectParticles3D::_update_uniform()
{
	Renderable::_update_uniform();

	_shader->uniform("inst_id", _inst_id);
}
auto Kasumi::ObjectParticles3D::ray_cast(const mRay3 &ray) const -> HinaPE::Geom::SurfaceRayIntersection3
{
	HinaPE::Geom::SurfaceRayIntersection3 res;
	const auto &verts_local = _mesh->_mesh->_verts_eigen4;
	const auto &idxs = _mesh->_mesh->_idxs_eigen;

	for (size_t i = 0; i < _poses.size(); ++i)
	{
		auto model = _poses[i].get_model_matrix()._m;
		auto t = (model * verts_local.transpose());
		Eigen::MatrixXd verts_world = t.transpose();

		Eigen::MatrixXd verts_world_3 = verts_world.block(0, 0, verts_world.rows(), 3);

		std::vector<igl::Hit> hits;
		bool intersected = igl::ray_mesh_intersect(ray._origin._v, ray._direction._v, verts_world_3, idxs, hits);
		if (intersected)
			for (auto const &hit: hits)
			{
				res.is_intersecting = true;
				auto distance = (static_cast<real>(hit.t) * ray._direction).length();
				if (distance < res.distance)
				{
					res.distance = distance;
					res.point = ray._origin + static_cast<real>(hit.t) * ray._direction;
					res.ID = this->ID;
					res.particleID = i;
					break;
				}
			}
	}
	return res;
}

// ==================== ObjectGrid3D ====================
Kasumi::ObjectGrid3D::ObjectGrid3D()
{
	NAME = "Grid";
	_shader = Shader::DefaultInstanceLineShader;
	_init();
}
void Kasumi::ObjectGrid3D::_init()
{
	std::vector<Pose> _poses;

	int scale = 15;
	for (int i = -scale; i < scale; ++i)
	{
		for (int j = -scale; j < scale; ++j)
		{
			for (int k = -scale; k < scale; ++k)
			{
				Pose pose;
				pose.position = 0.1 * mVector3(i, j, k);
				_poses.push_back(pose);
			}
		}
	}

	std::shared_ptr<Lines> _bbox_lines = std::make_shared<Lines>();
	_bbox_lines->clear();
	auto l = mVector3{-HinaPE::Constant::Half, -HinaPE::Constant::Half, -HinaPE::Constant::Half};
	auto u = mVector3{HinaPE::Constant::Half, HinaPE::Constant::Half, HinaPE::Constant::Half};

	mVector3 color = HinaPE::Color::PURPLE;

	// bounding box lines
	_bbox_lines->add(mVector3(l.x(), l.y(), l.z()), mVector3(u.x(), l.y(), l.z()), color);
	_bbox_lines->add(mVector3(u.x(), l.y(), l.z()), mVector3(u.x(), u.y(), l.z()), color);
	_bbox_lines->add(mVector3(u.x(), u.y(), l.z()), mVector3(l.x(), u.y(), l.z()), color);
	_bbox_lines->add(mVector3(l.x(), u.y(), l.z()), mVector3(l.x(), l.y(), l.z()), color);

	_bbox_lines->add(mVector3(l.x(), l.y(), u.z()), mVector3(u.x(), l.y(), u.z()), color);
	_bbox_lines->add(mVector3(u.x(), l.y(), u.z()), mVector3(u.x(), u.y(), u.z()), color);
	_bbox_lines->add(mVector3(u.x(), u.y(), u.z()), mVector3(l.x(), u.y(), u.z()), color);
	_bbox_lines->add(mVector3(l.x(), u.y(), u.z()), mVector3(l.x(), l.y(), u.z()), color);

	_bbox_lines->add(mVector3(l.x(), l.y(), l.z()), mVector3(l.x(), l.y(), u.z()), color);
	_bbox_lines->add(mVector3(u.x(), l.y(), l.z()), mVector3(u.x(), l.y(), u.z()), color);
	_bbox_lines->add(mVector3(u.x(), u.y(), l.z()), mVector3(u.x(), u.y(), u.z()), color);
	_bbox_lines->add(mVector3(l.x(), u.y(), l.z()), mVector3(l.x(), u.y(), u.z()), color);

	_grids = std::make_shared<InstancedLines>(_bbox_lines);

	for (auto &pose: _poses)
		_grids->_opt.instance_matrices.push_back(pose.get_model_matrix());
	_grids->_opt.dirty = true;
}
void Kasumi::ObjectGrid3D::_draw()
{
	if (_grids == nullptr) return;
	_grids->render(*_shader);
}
