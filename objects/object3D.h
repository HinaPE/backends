#ifndef KASUMI_OBJECT3D_H
#define KASUMI_OBJECT3D_H

// Copyright (c) 2023 Xayah Hina
// MPL-2.0 license

#include <utility>

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
	void set_color(const mVector3 &color);

protected:
	void _init(const std::string& MESH,const std::string& TEXTURE,const mVector3& COLOR = HinaPE::Color::MIKU);
	void _init(std::vector<Mesh::Vertex> &&vertices, std::vector<Mesh::Index> &&indices, std::map<std::string, std::vector<TexturePtr>> &&textures);
	void _draw() final;
	void _update_uniform() final;
	virtual void _update_surface() {}

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


class ObjectLines3DInstanced :
		public Renderable,
		public IDBase,
		public NameBase,
		public PoseBase,
		public VALID_CHECKER
{
public:
	void add(const mVector3 &start, const mVector3 &end, const mVector3 &color = HinaPE::Color::ORANGE);
	void clear();
	ObjectLines3DInstanced();

protected:
	void _init();
	void _draw() final;

private:
	InstancedLinesPtr _lines;
	std::vector<Pose> _poses;
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


class ObjectPoints3DInstanced:
		public Renderable,
		public IDBase,
		public NameBase,
		public InstancePosesBase,
		public VALID_CHECKER
{
public:
	[[deprecated("You don't need to use this function anymore, use track(std::vector<mVector3> *pos) instead")]]
	void add(const mVector3 &point, const mVector3 &color = HinaPE::Color::ORANGE);
	[[deprecated("You don't need to use this function anymore, use track(std::vector<mVector3> *pos) instead")]]
	void clear();
	auto ray_cast(const mRay3 & ray) const -> HinaPE::Geom::SurfaceRayIntersection3;
	void hide(bool value);
	int _inst_id;
	ObjectPoints3DInstanced();

protected:
	void _init();
	void _update();
	void _draw() final;
	void _update_uniform() final;

private:
	InstancedPointsPtr _points;
	bool _hidden = false;
	bool _random_color = false;
	real point_size = 3; // NOT used yet
};


class ObjectParticles3D :
		public Renderable,
		public IDBase,
		public NameBase,
		public InstancePosesBase,
		public VALID_CHECKER
{
public:
	auto ray_cast(const mRay3 & ray) const -> HinaPE::Geom::SurfaceRayIntersection3;
	void hide(bool value);
	int _inst_id;
	ObjectParticles3D();

protected:
	void _init(const std::string& MESH,const std::string& TEXTURE,const mVector3& COLOR = HinaPE::Color::ORANGE);
	void _update();
	void _draw() final;
	void _update_uniform() final;

	friend class Scene3D;
	void _switch_surface() const { _mesh->_opt.dirty = true; _mesh->_opt.render_surface = !_mesh->_opt.render_surface; }
	void _switch_wireframe() const { _mesh->_opt.dirty = true; _mesh->_opt.render_wireframe = !_mesh->_opt.render_wireframe; }
	void _switch_bbox() const { _mesh->_opt.dirty = true; _mesh->_opt.render_bbox = !_mesh->_opt.render_bbox; }

protected:
	InstancedMeshPtr _mesh;
	bool _hidden = false;
	bool _random_color = false;
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
};


using ObjectMesh3DPtr = std::shared_ptr<ObjectMesh3D>;
using ObjectLines3DPtr = std::shared_ptr<ObjectLines3D>;
using ObjectLines3DInstancedPtr = std::shared_ptr<ObjectLines3DInstanced>;
using ObjectPoints3DPtr = std::shared_ptr<ObjectPoints3D>;
using ObjectPoints3DInstancedPtr = std::shared_ptr<ObjectPoints3DInstanced>;
using ObjectParticles3DPtr = std::shared_ptr<ObjectParticles3D>;
using ObjectGrid3DPtr = std::shared_ptr<ObjectGrid3D>;
} // namespace Kasumi
// @formatter:on
#endif //KASUMI_OBJECT3D_H
