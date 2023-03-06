#ifndef KASUMI_OBJECT3D_H
#define KASUMI_OBJECT3D_H

// Copyright (c) 2023 Xayah Hina
// MPL-2.0 license

#include "backends/api.h"
#include "geom/collider3.h"

namespace Kasumi
{
// @formatter:off
class ObjectMesh3D :
		public Renderable,
		public IDBase,
		public NameBase,
		public PoseBase,
		public VALID_CHECKER,
		public HinaPE::Geom::RigidBodyCollider3
{
public:
	auto ray_cast(const mRay3 & ray) const -> HinaPE::Geom::SurfaceRayIntersection3;

protected:
	void _init(const std::string& MESH,const std::string& TEXTURE,const mVector3& COLOR = HinaPE::Color::CYAN);
	void _draw() final;
	void _update_uniform() final;

	friend class Scene3D;
	void _switch_surface() const { _mesh->_opt.dirty = true;_mesh->_opt.render_surface = !_mesh->_opt.render_surface; }
	void _switch_wireframe() const { _mesh->_opt.dirty = true;_mesh->_opt.render_wireframe = !_mesh->_opt.render_wireframe; }
	void _switch_bbox() const { _mesh->_opt.dirty = true;_mesh->_opt.render_bbox = !_mesh->_opt.render_bbox; }

protected:
	MeshPtr _mesh; // verts & idxs

	void INSPECT() override;
	void VALID_CHECK() const final;
};


class ObjectLines3D :
		public Renderable,
		public IDBase,
		public NameBase,
		public PoseBase,
		public VALID_CHECKER
{
public:
	static void Init();
	static std::shared_ptr<ObjectLines3D> DefaultLines;

	void add(const mVector3 &start, const mVector3 &end, const mVector3 &color = HinaPE::Color::PURPLE);
	void clear();
	ObjectLines3D();

protected:
	void _draw() final;
	void _update_uniform() final;

private:
	LinesPtr _lines;
};


class ObjectPoints3D :
		public Renderable,
		public IDBase,
		public NameBase,
		public PoseBase,
		public VALID_CHECKER
{
public:
	static void Init();
	static std::shared_ptr<ObjectPoints3D> DefaultPoints;
	ObjectPoints3D();

	void add(const mVector3 &point, const mVector3 &color = HinaPE::Color::PURPLE);
	void clear();

protected:
	void _draw() final;
	void _update_uniform() final;

private:
	PointsPtr _points;
};


class ObjectParticles3D :
		public Renderable,
		public IDBase,
		public NameBase,
		public PoseBase,
		public VALID_CHECKER
{
public:
	void _switch_surface() const { _mesh->_opt.dirty = true; _mesh->_opt.render_surface = !_mesh->_opt.render_surface; }
	void _switch_wireframe() const { _mesh->_opt.dirty = true; _mesh->_opt.render_wireframe = !_mesh->_opt.render_wireframe; }
	void _switch_bbox() const { _mesh->_opt.dirty = true; _mesh->_opt.render_bbox = !_mesh->_opt.render_bbox; }
	ObjectParticles3D();

protected:
	void _init(const std::string& MESH,const std::string& TEXTURE,const mVector3& COLOR = HinaPE::Color::CYAN);
	void _update();
	void _draw() final;
	void _update_uniform() final;

private:
	InstancedMeshPtr _mesh;
	std::vector<Pose> _poses;
	bool _dirty = true;
};


class ObjectGrid3D :
		public Renderable,
		public IDBase,
		public NameBase,
		public PoseBase,
		public VALID_CHECKER
{
public:
	ObjectGrid3D();

protected:
	void _init();
	void _draw() final;

private:
	InstancedLinesPtr _grids;
	std::vector<Pose> _poses;
};


using ObjectMesh3DPtr = std::shared_ptr<ObjectMesh3D>;
using ObjectLines3DPtr = std::shared_ptr<ObjectLines3D>;
using ObjectPoints3DPtr = std::shared_ptr<ObjectPoints3D>;
using ObjectParticles3DPtr = std::shared_ptr<ObjectParticles3D>;
using ObjectGrid3DPtr = std::shared_ptr<ObjectGrid3D>;
} // namespace Kasumi
// @formatter:on
#endif //KASUMI_OBJECT3D_H
