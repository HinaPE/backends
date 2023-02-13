#ifndef KASUMI_MESH_H
#define KASUMI_MESH_H

// Copyright (c) 2023 Xayah Hina
// MPL-2.0 license

// Dependency:
// - Math Backend
// - Shader Backend
// - Texture Backend

#include "math_api.h"
#include "shader.h"
#include "texture.h"

#include <map>

namespace Kasumi
{
//!
//! This is an ENCAPSULATED class.
//! Any access is not permitted, except for the friend classes.
//!
class UniversalMesh final
{
protected:
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

public:
	struct Opt
	{
		bool colored = false;
		bool textured = false;
		bool instanced = false;
		int instance_count;
	} _opt;
	void render(const Shader &shader);
	inline auto vertices() -> std::vector<Vertex> & { return _verts; }
	inline auto indices() const -> const std::vector<Index> & { return _idxs; }
	inline void mark_dirty() { _dirty = true; }

private:
	unsigned int _vao, _vbo, _ebo;
	std::vector<Vertex> _verts;
	std::vector<Index> _idxs;
	std::map<std::string, std::vector<TexturePtr>> _textures;
	bool _dirty;

//! ==================== Geometry Info ====================
private:
	mVector3 _center_point;
	mBBox3 _bbox;

public:
	friend class Model;
	UniversalMesh(const std::string &primitive_name, const std::string &texture_name);
	UniversalMesh(const std::string &primitive_name, const mVector3 &color);
	UniversalMesh(std::vector<Vertex> &&vertices, std::vector<Index> &&indices, std::map<std::string, std::vector<TexturePtr>> &&textures = {});

public:
	UniversalMesh(const UniversalMesh &src) = delete;
	UniversalMesh(UniversalMesh &&src) noexcept = default;
	void operator=(const UniversalMesh &src) = delete;
	auto operator=(UniversalMesh &&src) noexcept -> UniversalMesh & = default;
	~UniversalMesh();

private:
	void _init(std::vector<Vertex> &&vertices, std::vector<Index> &&indices);
	void _load_primitive(const std::string &primitive_name, std::vector<Kasumi::UniversalMesh::Vertex> &vertices, std::vector<unsigned int> &indices, const mVector3 &color = HinaPE::Color::NO_COLORS);
	void _update();
};
using UniversalMeshPtr = std::shared_ptr<UniversalMesh>;

class Lines final
{
public:
	struct Vertex
	{
		mVector3 position;
		mVector3 color;
	};
	Lines();
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

private:
	void _init();
	void _update();

private:
	unsigned int _vao, _vbo;
	std::vector<Vertex> _lines; // start, end
};
using LinesPtr = std::shared_ptr<Lines>;
} // namespace Kasumi
#endif //KASUMI_MESH_H
