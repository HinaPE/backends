#include "bunny.h"

#include "igl/random_points_on_mesh.h"

Kasumi::BunnyObject::BunnyObject()
{
	NAME = "Bunny";
	_shader = Shader::DefaultMeshShader;
	_init("bunny", "");
	_mesh->centralize();
	load_surface(this);

	std::vector<mVector3> vertices;
	std::vector<unsigned int> indices;

	for (auto &v: _mesh->vertices())
		vertices.push_back(v.position);
	for (auto &i: _mesh->indices())
		indices.push_back(i);
	reload(vertices, indices);

	_transform = mTransform3(POSE.position, mQuaternion(POSE.euler));
}

struct Boundary
{
	std::vector<cs224::Vector3f> positions;
	std::vector<cs224::Vector3f> normals;
};

auto Kasumi::BunnyObject::generate_surface() const -> std::vector<mVector3>
{
	// igl impl
	auto V = _mesh->_verts_eigen3;
	auto F = _mesh->_idxs_eigen;

	Eigen::MatrixXd P;
	{
		Eigen::VectorXi I;
		Eigen::SparseMatrix<double> B;
		igl::random_points_on_mesh(2000,V,F,B,I);
		P = B*V;
	}

	std::vector<mVector3> res;
	for (int i = 0; i < P.rows(); ++i)
		res.emplace_back(P(i, 0), P(i, 1), P(i, 2));
	return res;

//
//	// temporary implementation
//
//	std::vector<mVector3> vertices;
//	for (auto &v: _mesh->vertices())
//		vertices.push_back(v.position);
//	auto indices = _mesh->indices();
//
//	HinaPE::Geom::TriangleMeshSurface m;
//	m.reload(vertices, indices);
//	auto normals = m.normals_per_vertex();
//
//	cs224::Mesh mesh;
//	for (int i = 0; i < vertices.size(); ++i)
//		mesh.addVertex(cs224::Vector3f(vertices[i].x(), vertices[i].y(), vertices[i].z()), cs224::Vector3f(normals[i].x(), normals[i].y(), normals[i].z()), cs224::Vector2f(0, 0));
//
//	for (int i = 0; i < indices.size(); i += 3)
//		mesh.addTriangle(indices[i], indices[i + 1], indices[i + 2]);
//
//	real particleRadius = 0.029;
//	int cells = 100;
//
//	float density = 1.f / (PI * cs224::pow2(particleRadius));
//
//	// compute bounds of mesh and expand by 10%
//	cs224::Box3f bounds = mesh.bound();
//	bounds = bounds.expanded(bounds.extents());
//
//	// compute cell and grid size for signed distance field
//	float cellSize = bounds.extents()[bounds.majorAxis()] / cells;
//
//	cs224::Vector3i size(int(std::ceil(bounds.extents().x() / cellSize)), int(std::ceil(
//			bounds.extents().y() / cellSize)), int(std::ceil(bounds.extents().z() / cellSize)));
//
//	cs224::VoxelGrid<float> sdf(size);
//	sdf.setOrigin(bounds.min);
//	sdf.setCellSize(cellSize);
//
//	// build signed distance field
//	cs224::SDF::build(mesh, sdf);
//
//	// generate initial point distribution
//	Boundary ret;
//	float totalArea = 0.f;
//
//	for (int i = 0; i < mesh.triangles().cols(); ++i)
//	{
//		const cs224::Vector3f &p0 = mesh.vertices().col(mesh.triangles()(0, i));
//		const cs224::Vector3f &p1 = mesh.vertices().col(mesh.triangles()(1, i));
//		const cs224::Vector3f &p2 = mesh.vertices().col(mesh.triangles()(2, i));
//
//		cs224::Vector3f v1 = p1 - p0;
//		cs224::Vector3f v2 = p2 - p0;
//
//		float area = 0.5f * std::abs(v1.cross(v2).norm());
//		totalArea += area;
//
//		int numi = int(std::floor(density * area));
//
//		auto samplePoint = [&]()
//		{
//			float s = cs224::randomFloat();
//			float t = cs224::randomFloat();
//			float ssq = std::sqrt(s);
//			ret.positions.emplace_back(p0 + v1 * t * ssq + v2 * (1.f - ssq));
//		};
//
//		// sample particles on the triangle randomly
//		for (int num = 0; num < numi; ++num)
//		{
//			samplePoint();
//		}
//	}
//
//	// choose radius to support roughly 10 neighbour particles
//	float radius = std::sqrt(totalArea / ret.positions.size() * 10.f / PI);
//	float radius_sq = cs224::pow2(radius);
//
//	cs224::Grid grid;
//	grid.init(bounds, bounds.extents().maxCoeff() / 128.f);
//
//	// keep doing 10 times to smooth particle positions
//	for (int iteration = 0; iteration < 10; ++iteration)
//	{
//		int count = 0;
//		std::vector<cs224::Vector3f> velocities(ret.positions.size(), cs224::Vector3f());
//
//		grid.update(ret.positions, [&](size_t i, size_t j)
//		{
//			std::swap(ret.positions[i], ret.positions[j]);
//		});
//
//		// relax positions
//		for (size_t i = 0; i < ret.positions.size(); ++i)
//		{
//			grid.lookup(ret.positions[i], radius, [&](size_t j)
//			{
//				if (i == j) return true;
//				cs224::Vector3f r = ret.positions[j] - ret.positions[i];
//				float rsq = r.squaredNorm();
//
//				if (rsq < radius_sq)
//				{
//					r *= (1.f / std::sqrt(rsq));
//					float weight = 0.002f * cs224::pow3(1.f - rsq / radius_sq);
//					velocities[i] -= weight * r;
//					count += 1;
//				}
//
//				return true;
//			});
//
//			ret.positions[i] += velocities[i];
//		}
//
//		// reproject to surface
//		for (size_t i = 0; i < ret.positions.size(); ++i)
//		{
//			cs224::Vector3f p = sdf.toVoxelSpace(ret.positions[i]);
//			cs224::Vector3f n = sdf.gradient(p).normalized();
//			ret.positions[i] -= sdf.trilinear(p) * n;
//		}
//	}
//
//	// compute normals
//	ret.normals.resize(ret.positions.size());
//	for (size_t i = 0; i < ret.positions.size(); ++i)
//	{
//		ret.normals[i] = sdf.gradient(sdf.toVoxelSpace(ret.positions[i])).normalized();
//	}
//
//	std::vector<mVector3> res;
//	for (auto v: ret.positions)
//		res.emplace_back(v.x(), v.y(), v.z());
//
//	return res;
}
void Kasumi::BunnyObject::_update_surface()
{
	ObjectMesh3D::_update_surface();
}
