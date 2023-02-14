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

Kasumi::UniversalMesh::UniversalMesh(const std::string &primitive_name, const std::string &texture_name) : _vao(0), _vbo(0), _ebo(0), _dirty(true)
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
Kasumi::UniversalMesh::UniversalMesh(std::vector<Vertex> &&vertices, std::vector<Index> &&indices, std::map<std::string, std::vector<TexturePtr>> &&textures) : _vao(0), _vbo(0), _ebo(0), _dirty(true)
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
Kasumi::UniversalMesh::UniversalMesh(const std::string &primitive_name, const mVector3 &color) : _vao(0), _vbo(0), _ebo(0), _dirty(true)
{
	std::vector<Vertex> vertices;
	std::vector<Index> indices;
	_load_primitive(primitive_name, vertices, indices, color);
	_opt.colored = true;
	_init(std::move(vertices), std::move(indices));
}

Kasumi::UniversalMesh::~UniversalMesh()
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

void Kasumi::UniversalMesh::render(const Kasumi::Shader &shader)
{
	if (_dirty)
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
	if (_opt.instanced)
		glDrawElementsInstanced(GL_TRIANGLES, _idxs.size(), GL_UNSIGNED_INT, 0, _opt.instance_count);
	else
		glDrawElements(GL_TRIANGLES, (GLuint) _idxs.size(), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}
// ================================================== Public Methods ==================================================



// ================================================== Private Methods ==================================================

void Kasumi::UniversalMesh::_init(std::vector<Vertex> &&vertices, std::vector<Index> &&indices)
{
	_verts = std::move(vertices);
	_idxs = std::move(indices);

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

	_center_point = mVector3(0.0f, 0.0f, 0.0f);
	for (auto &v: _verts)
		_center_point += v.position;
	_center_point /= static_cast<float>(_verts.size());

	_bbox.reset();
	for (auto &v: _verts)
		_bbox.merge(v.position);
	_dirty = true;
}
void Kasumi::UniversalMesh::_update()
{
	glBindVertexArray(_vao);

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * _verts.size(), &_verts[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index) * _idxs.size(), &_idxs[0], GL_DYNAMIC_DRAW);

	glBindVertexArray(0);
	_dirty = false;
}
void Kasumi::UniversalMesh::_load_primitive(const std::string &primitive_name, std::vector<Kasumi::UniversalMesh::Vertex> &vertices, std::vector<unsigned int> &indices, const mVector3 &color)
{
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(std::string(BackendsModelDir) + primitive_name + ".obj", aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode || scene->mNumMeshes > 1 || scene->mRootNode->mNumChildren > 1 /* primitive type should be only one mesh*/)
		return;

	auto *mesh = scene->mMeshes[0];
	for (int i = 0; i < mesh->mNumVertices; ++i)
	{
		Kasumi::UniversalMesh::Vertex v;
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
	glDrawArrays(GL_LINES, 0, (GLsizei) _lines.size());
	glBindVertexArray(0);
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
void Kasumi::Lines::clear()
{
	_lines.clear();
	_opt.dirty = true;
}

// ================================================== Lines ==================================================