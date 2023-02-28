#include "../mesh.h"

#include "glad/glad.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#ifdef HINAPE_DOUBLE
#define GL_REAL GL_DOUBLE
#else
#define GL_REAL GL_FLOAT
#endif

Kasumi::Mesh::Mesh(const std::string &primitive_name, const std::string &texture_name) : _vao(0), _vbo(0), _ebo(0)
{
	std::vector<Vertex> vertices;
	std::vector<Index> indices;
	std::vector<TexturePtr> diffuse_textures;
	_load_primitive(primitive_name, vertices, indices);

	diffuse_textures.push_back(std::make_shared<Kasumi::Texture>(std::string(BackendsTextureDir) + texture_name));
	_textures["diffuse"] = diffuse_textures;
	_opt.textured = true;
	_init(std::move(vertices), std::move(indices));
}
Kasumi::Mesh::Mesh(std::vector<Vertex> &&vertices, std::vector<Index> &&indices, std::map<std::string, std::vector<TexturePtr>> &&textures) : _vao(0), _vbo(0), _ebo(0)
{
	if (!textures.empty())
	{
		for (auto &pair: textures)
			if (!pair.second.empty())
				_opt.textured = true;
		_textures = std::move(textures);
	} else
		_opt.textured = false;
	_init(std::move(vertices), std::move(indices));
}
Kasumi::Mesh::Mesh(const std::string &primitive_name, const mVector3 &color) : _vao(0), _vbo(0), _ebo(0)
{
	std::vector<Vertex> vertices;
	std::vector<Index> indices;
	_load_primitive(primitive_name, vertices, indices, color);
	_opt.colored = true;
	_init(std::move(vertices), std::move(indices));
}

Kasumi::Mesh::~Mesh()
{
	glDeleteBuffers(1, &_vao);
	glDeleteBuffers(1, &_vbo);
	glDeleteVertexArrays(1, &_ebo);
	_vao = _vbo = _ebo = 0;
	_textures.clear();

#ifdef HINAPE_DEBUG
	std::cout << "delete textured mesh" << std::endl;
#endif
}

// ================================================== Public Methods ==================================================

void Kasumi::Mesh::render(const Kasumi::Shader &shader)
{
	if (_opt.dirty)
		_update();

	_opt.depth_test ? glEnable(GL_DEPTH_TEST)
					: glDisable(GL_DEPTH_TEST);

	_opt.stencil_test ? glEnable(GL_STENCIL_TEST)
					  : glDisable(GL_STENCIL_TEST);

	_opt.cull_face ? glEnable(GL_CULL_FACE)
				   : glDisable(GL_CULL_FACE);

	_opt.blend ? glEnable(GL_BLEND)
			   : glDisable(GL_BLEND);

	_opt.render_wireframe ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)
						  : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	if (_opt.render_surface)
	{
		shader.use();

		int texture_index = 0;
		auto diffuse_textures = _textures["diffuse"];
		switch (diffuse_textures.size())
		{
			case 1:
			{
				diffuse_textures[0]->bind(texture_index);
				shader.uniform("diffuse_texture_num", 1);
				shader.uniform("texture_diffuse1", texture_index);
				++texture_index;
			}
				break;
			case 2:
			{
				diffuse_textures[0]->bind(texture_index);
				diffuse_textures[1]->bind(texture_index + 1);
				shader.uniform("diffuse_texture_num", 2);
				shader.uniform("texture_diffuse1", texture_index);
				shader.uniform("texture_diffuse2", texture_index + 1);
				++texture_index;
				++texture_index;
			}
				break;
			default: // NOT SUPPORT FOR MORE DIFFUSE TEXTURES NOW
				shader.uniform("diffuse_texture_num", 0);
				break;
		}
		auto specular_textures = _textures["specular"];
		if (!specular_textures.empty())
		{
			specular_textures[0]->bind(texture_index);
			shader.uniform("specular_texture_num", 1);
			shader.uniform("texture_specular1", texture_index);
			texture_index++;
		} else
			shader.uniform("specular_texture_num", 0);
		auto normal_textures = _textures["normal"];
		if (!normal_textures.empty())
		{
			normal_textures[0]->bind(texture_index);
			shader.uniform("normal_texture_num", 1);
			shader.uniform("texture_normal1", texture_index);
			texture_index++;
		} else
			shader.uniform("normal_texture_num", 0);
		auto height_textures = _textures["height"];
		if (!height_textures.empty())
		{
			height_textures[0]->bind(texture_index);
			shader.uniform("height_texture_num", 1);
			shader.uniform("texture_height1", texture_index);
			texture_index++;
		} else
			shader.uniform("height_texture_num", 0);

		shader.uniform("is_colored", _opt.colored);
		shader.uniform("is_textured", _opt.textured);

		glBindVertexArray(_vao);
		if (_opt.instanced && _opt.instance_count > 0)
			glDrawElementsInstanced(GL_TRIANGLES, _idxs.size(), GL_UNSIGNED_INT, 0, _opt.instance_count);
		else
			glDrawElements(GL_TRIANGLES, (GLuint) _idxs.size(), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
	}

	if (_opt.render_bbox)
	{
		_bbox_lines->render(*Shader::DefaultLineShader);
	}
}
// ================================================== Public Methods ==================================================



// ================================================== Private Methods ==================================================

void Kasumi::Mesh::_init(std::vector<Vertex> &&vertices, std::vector<Index> &&indices)
{
	_verts = std::move(vertices);
	_idxs = std::move(indices);
	_bbox_lines = std::make_shared<Kasumi::Lines>();

	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vbo);
	glGenBuffers(1, &_ebo);

	glBindVertexArray(_vao);

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);

	glVertexAttribPointer(0, 3, GL_REAL, GL_FALSE, sizeof(Vertex), (GLvoid *) offsetof(Vertex, position)); // location = 0, position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_REAL, GL_FALSE, sizeof(Vertex), (GLvoid *) offsetof(Vertex, normal)); // location = 1, normal
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_REAL, GL_FALSE, sizeof(Vertex), (GLvoid *) offsetof(Vertex, tex_coord)); // location = 2, tex_coord
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 3, GL_REAL, GL_FALSE, sizeof(Vertex), (GLvoid *) offsetof(Vertex, color)); // location = 3, color
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(4, 3, GL_REAL, GL_FALSE, sizeof(Vertex), (GLvoid *) offsetof(Vertex, tangent)); // location = 4, tangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(5, 3, GL_REAL, GL_FALSE, sizeof(Vertex), (GLvoid *) offsetof(Vertex, bi_tangent)); // location = 5, bi_tangent
	glEnableVertexAttribArray(5);
	glVertexAttribIPointer(6, 1, GL_UNSIGNED_INT, sizeof(Vertex), (GLvoid *) offsetof(Vertex, id)); // location = 6, id
	glEnableVertexAttribArray(6);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);

	glBindVertexArray(0);
	_opt.dirty = true;
}
void Kasumi::Mesh::_update()
{
	glBindVertexArray(_vao);

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * _verts.size(), &_verts[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index) * _idxs.size(), &_idxs[0], GL_DYNAMIC_DRAW);

	glBindVertexArray(0);

	_center_point = mVector3(0.0f, 0.0f, 0.0f);
	for (auto &v: _verts)
		_center_point += v.position;
	_center_point /= static_cast<float>(_verts.size());

	_bbox.reset();
	for (auto &v: _verts)
		_bbox.merge(v.position);

	_bbox_lines->clear();
	auto l = _bbox._lower_corner;
	auto u = _bbox._upper_corner;

	// bounding box lines
	_bbox_lines->add(mVector3(l.x(), l.y(), l.z()), mVector3(u.x(), l.y(), l.z()), _opt.bbox_color);
	_bbox_lines->add(mVector3(u.x(), l.y(), l.z()), mVector3(u.x(), u.y(), l.z()), _opt.bbox_color);
	_bbox_lines->add(mVector3(u.x(), u.y(), l.z()), mVector3(l.x(), u.y(), l.z()), _opt.bbox_color);
	_bbox_lines->add(mVector3(l.x(), u.y(), l.z()), mVector3(l.x(), l.y(), l.z()), _opt.bbox_color);

	_bbox_lines->add(mVector3(l.x(), l.y(), u.z()), mVector3(u.x(), l.y(), u.z()), _opt.bbox_color);
	_bbox_lines->add(mVector3(u.x(), l.y(), u.z()), mVector3(u.x(), u.y(), u.z()), _opt.bbox_color);
	_bbox_lines->add(mVector3(u.x(), u.y(), u.z()), mVector3(l.x(), u.y(), u.z()), _opt.bbox_color);
	_bbox_lines->add(mVector3(l.x(), u.y(), u.z()), mVector3(l.x(), l.y(), u.z()), _opt.bbox_color);

	_bbox_lines->add(mVector3(l.x(), l.y(), l.z()), mVector3(l.x(), l.y(), u.z()), _opt.bbox_color);
	_bbox_lines->add(mVector3(u.x(), l.y(), l.z()), mVector3(u.x(), l.y(), u.z()), _opt.bbox_color);
	_bbox_lines->add(mVector3(u.x(), u.y(), l.z()), mVector3(u.x(), u.y(), u.z()), _opt.bbox_color);
	_bbox_lines->add(mVector3(l.x(), u.y(), l.z()), mVector3(l.x(), u.y(), u.z()), _opt.bbox_color);

	_opt.dirty = false;
}
void Kasumi::Mesh::_load_primitive(const std::string &primitive_name, std::vector<Kasumi::Mesh::Vertex> &vertices, std::vector<unsigned int> &indices, const mVector3 &color)
{
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(std::string(BackendsModelDir) + primitive_name + ".obj", aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode || scene->mNumMeshes > 1 || scene->mRootNode->mNumChildren > 1 /* primitive type should be only one mesh*/)
		throw std::runtime_error("Failed to load primitive: " + primitive_name);

	auto *mesh = scene->mMeshes[0];
	for (int i = 0; i < mesh->mNumVertices; ++i)
	{
		Kasumi::Mesh::Vertex v;
		v.position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
		v.color = color;
		if (mesh->HasNormals())
			v.normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
		if (mesh->HasTextureCoords(0))
			v.tex_coord = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
		if (mesh->HasTangentsAndBitangents())
		{
			v.tangent = {mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z};
			v.bi_tangent = {mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z};
		}
		v.id = i; // TODO: auto id
		vertices.emplace_back(std::move(v));
	}
	for (int i = 0; i < mesh->mNumFaces; ++i)
		for (int j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
			indices.emplace_back(mesh->mFaces[i].mIndices[j]);
}

// ================================================== Private Methods ==================================================



// ================================================== Lines ==================================================
Kasumi::Lines::Lines() : _vao(0), _vbo(0) { _init(); }
void Kasumi::Lines::add(const mVector3 &start, const mVector3 &end, const mVector3 &color)
{
	_lines.emplace_back(Vertex{start, color});
	_lines.emplace_back(Vertex{end, color});
	_opt.dirty = true;
}
void Kasumi::Lines::render(const Kasumi::Shader &shader)
{
	if (_lines.empty())
		return;

	shader.use();
	shader.uniform("opacity", _opt._opacity);

	if (_opt.dirty)
		_update();

	glLineWidth(_opt.thickness);
	if (_opt.smooth)
		glEnable(GL_LINE_SMOOTH);
	else
		glDisable(GL_LINE_SMOOTH);

	glBindVertexArray(_vao);
	if (_opt.instanced && _opt.instance_count > 0)
		glDrawArraysInstanced(GL_LINES, 0, (GLsizei) _lines.size(), _opt.instance_count);
	else
		glDrawArrays(GL_LINES, 0, (GLsizei) _lines.size());
	glBindVertexArray(0);
}
void Kasumi::Lines::clear()
{
	_lines.clear();
	_opt.dirty = true;
}
auto Kasumi::Lines::lines() -> std::vector<Vertex> &
{
	_opt.dirty = true;
	return _lines;
}
void Kasumi::Lines::_init()
{
	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vbo);
	glBindVertexArray(_vao);

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glVertexAttribPointer(0, 3, GL_REAL, GL_FALSE, sizeof(Vertex), (GLvoid *) offsetof(Vertex, position)); // location = 0, position
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_REAL, GL_FALSE, sizeof(Vertex), (GLvoid *) offsetof(Vertex, color)); // location = 1, color
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}
void Kasumi::Lines::_update()
{
	glBindVertexArray(_vao);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * _lines.size(), &_lines[0], GL_DYNAMIC_DRAW);
	glBindVertexArray(0);

	_opt.dirty = false;
}

// ================================================== Lines ==================================================



// ================================================== InstancedMesh ==================================================
Kasumi::InstancedMesh::InstancedMesh(Kasumi::MeshPtr mesh) : _mesh(std::move(mesh)), _instanceVBO(0)
{
	glGenBuffers(1, &_instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _instanceVBO);

	glBindVertexArray(_mesh->_vao);
	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 4, GL_REAL, GL_FALSE, sizeof(mMatrix4x4), (void *) 0);
	glEnableVertexAttribArray(8);
	glVertexAttribPointer(8, 4, GL_REAL, GL_FALSE, sizeof(mMatrix4x4), (void *) (sizeof(mVector4)));
	glEnableVertexAttribArray(9);
	glVertexAttribPointer(9, 4, GL_REAL, GL_FALSE, sizeof(mMatrix4x4), (void *) (2 * sizeof(mVector4)));
	glEnableVertexAttribArray(10);
	glVertexAttribPointer(10, 4, GL_REAL, GL_FALSE, sizeof(mMatrix4x4), (void *) (3 * sizeof(mVector4)));

	glVertexAttribDivisor(7, 1);
	glVertexAttribDivisor(8, 1);
	glVertexAttribDivisor(9, 1);
	glVertexAttribDivisor(10, 1);

	glBindVertexArray(0);

	_mesh->_opt.instanced = true;
	_mesh->_opt.instance_count = static_cast<int>(_opt.instance_matrices.size());
}
void Kasumi::InstancedMesh::_update()
{
	if (!_opt.dirty && _opt.instance_matrices.empty())
		return;

	glBindBuffer(GL_ARRAY_BUFFER, _instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, _opt.instance_matrices.size() * sizeof(mMatrix4x4), &_opt.instance_matrices[0], GL_DYNAMIC_DRAW);

	_mesh->_opt.render_surface = _opt.render_surface;
	_mesh->_opt.render_wireframe = _opt.render_wireframe;
	_mesh->_opt.render_bbox = _opt.render_bbox;
	_mesh->_opt.instance_count = static_cast<int>(_opt.instance_matrices.size());

	_opt.dirty = false;
}
void Kasumi::InstancedMesh::render(const Kasumi::Shader &shader)
{
	if (_opt.instance_matrices.empty())
		return;

	if (_opt.dirty)
		_update();

	_mesh->render(shader);
}
// ================================================== InstancedMesh ==================================================

Kasumi::InstancedLines::InstancedLines(Kasumi::LinesPtr lines) : _lines(std::move(lines)), _instanceVBO(0)
{
	glGenBuffers(1, &_instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _instanceVBO);

	glBindVertexArray(_lines->_vao);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_REAL, GL_FALSE, sizeof(mMatrix4x4), (void *) 0);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_REAL, GL_FALSE, sizeof(mMatrix4x4), (void *) (sizeof(mVector4)));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_REAL, GL_FALSE, sizeof(mMatrix4x4), (void *) (2 * sizeof(mVector4)));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_REAL, GL_FALSE, sizeof(mMatrix4x4), (void *) (3 * sizeof(mVector4)));

	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);

	glBindVertexArray(0);

	_lines->_opt.instanced = true;
	_lines->_opt.instance_count = static_cast<int>(_opt.instance_matrices.size());
}
void Kasumi::InstancedLines::render(const Kasumi::Shader &shader)
{
	if (_opt.instance_matrices.empty())
		return;

	if (_opt.dirty)
		_update();

	_lines->render(shader);
}
void Kasumi::InstancedLines::_update()
{
	if (!_opt.dirty && _opt.instance_matrices.empty())
		return;

	glBindBuffer(GL_ARRAY_BUFFER, _instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, _opt.instance_matrices.size() * sizeof(mMatrix4x4), &_opt.instance_matrices[0], GL_DYNAMIC_DRAW);

	_lines->_opt.instance_count = static_cast<int>(_opt.instance_matrices.size());

	_opt.dirty = false;
}
Kasumi::Points::Points() : _vao(0), _vbo(0) { _init(); }
auto Kasumi::Points::points() -> std::vector<Vertex> & { return _points; }
void Kasumi::Points::_init()
{
	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vbo);
	glBindVertexArray(_vao);

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glVertexAttribPointer(0, 3, GL_REAL, GL_FALSE, sizeof(Vertex), (GLvoid *) offsetof(Vertex, position)); // location = 0, position
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_REAL, GL_FALSE, sizeof(Vertex), (GLvoid *) offsetof(Vertex, color)); // location = 1, color
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}
void Kasumi::Points::_update()
{
	glBindVertexArray(_vao);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * _points.size(), &_points[0], GL_DYNAMIC_DRAW);
	glBindVertexArray(0);

	_opt.dirty = false;
}
void Kasumi::Points::add(const mVector3 &p, const mVector3 &color)
{
	_points.emplace_back(Vertex{p, color});
	_opt.dirty = true;
}
void Kasumi::Points::render(const Kasumi::Shader &shader)
{
	if (_points.empty())
		return;

	shader.use();
	shader.uniform("opacity", 1.f);

	if (_opt.dirty)
		_update();

	glPointSize(10);

	glBindVertexArray(_vao);
	glDrawArrays(GL_POINTS, 0, (GLsizei) _points.size());
	glBindVertexArray(0);
}
void Kasumi::Points::clear()
{
	_points.clear();
	_opt.dirty = true;
}
