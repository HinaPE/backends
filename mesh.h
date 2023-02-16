#ifndef BACKENDS_MESH_H
#define BACKENDS_MESH_H

// Copyright (c) 2023 Xayah Hina
// MPL-2.0 license

#include "common.h"
#include "shader.h"
#include "texture.h"

namespace Kasumi
{
class Lines;
class Mesh final
{
public:
	struct Vertex
	{
		mVector3 position;
		mVector3 normal;
		mVector2 tex_coord;
		mVector3 color;
		mVector3 tangent;
		mVector3 bi_tangent;
		unsigned int id;
	};
	using Index = unsigned int;

	void render(const Shader &shader);
	inline auto vertices() -> std::vector<Vertex> & { _dirty = true; return _verts; }
	inline auto indices() const -> const std::vector<Index> & { return _idxs; }

public:
	struct Opt
	{
		bool colored = false;
		bool textured = false;
		bool instanced = false;
		int instance_count;

		// rendering options
		bool render_surface = true;
		bool render_wireframe = false;
		bool render_bbox = false;
		bool depth_test = true;
		bool stencil_test = false;
		bool cull_face = false;
		bool blend = true;

		// bounding box options
		mVector3 bbox_color = {0.f, 0.f, 0.f};

		// line model
		bool line_model = false;
	} _opt;
	Mesh(const std::string &primitive_name, const std::string &texture_name);
	Mesh(const std::string &primitive_name, const mVector3 &color);
	Mesh(std::vector<Vertex> &&vertices, std::vector<Index> &&indices, std::map<std::string, std::vector<TexturePtr>> &&textures = {});
	~Mesh();

private:
	void _init(std::vector<Vertex> &&vertices, std::vector<Index> &&indices);
	void _load_primitive(const std::string &primitive_name, std::vector<Kasumi::Mesh::Vertex> &vertices, std::vector<unsigned int> &indices, const mVector3 &color = HinaPE::Color::NO_COLORS);
	void _update();

private:
	friend class InstancedMesh;
	unsigned int _vao, _vbo, _ebo;
	std::vector<Vertex> _verts;
	std::vector<Index> _idxs;
	std::map<std::string, std::vector<TexturePtr>> _textures;
	bool _dirty;

	// geometry
	mVector3 _center_point;
	mBBox3 _bbox;
	std::shared_ptr<Lines> _bbox_lines;
};
using MeshPtr = std::shared_ptr<Mesh>;

class InstancedMesh final
{
public:
	void render(const Shader &shader);

public:
	struct Opt
	{
		bool dirty = true;
		bool render_surface = true;
		bool render_wireframe = false;
		std::vector<mMatrix4x4> instance_matrices;
	} _opt;
	explicit InstancedMesh(MeshPtr mesh);

private:
	void _update();
	MeshPtr _mesh;
	unsigned int _instanceVBO;
};
using InstancedMeshPtr = std::shared_ptr<InstancedMesh>;

class Lines final : public HinaPE::CopyDisable
{
public:
	struct Vertex
	{
		mVector3 position;
		mVector3 color;
	};
	auto lines() -> std::vector<Vertex> &;
	void add(const mVector3 &start, const mVector3 &end, const mVector3 &color = HinaPE::Color::PURPLE);
	void render(const Shader &shader);
	void clear();

public:
	struct Opt
	{
		bool dirty = true;
		bool smooth = true;
		float thickness = 1.0f; // NO USE FOR OpenGL 3.0
		float _opacity = 1.0;
	} _opt;
	Lines();

private:
	void _init();
	void _update();

private:
	unsigned int _vao, _vbo;
	std::vector<Vertex> _lines; // start, end
};
using LinesPtr = std::shared_ptr<Lines>;

class Points final : public HinaPE::CopyDisable
{
};
} // namespace Kasumi

#endif //BACKENDS_MESH_H
