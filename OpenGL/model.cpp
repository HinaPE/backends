#include "glad/glad.h"
#include "../model.h"

#include <map>
#include <utility>
#include <algorithm>

Kasumi::Model::Model(const std::string &model_path) : _path(model_path), _lines(std::make_shared<Lines>()) { load(model_path); }
Kasumi::Model::Model(const std::string &primitive_name, const std::string &texture_name) : _lines(std::make_shared<Lines>()) { load(std::make_shared<UniversalMesh>(primitive_name, texture_name)); }
Kasumi::Model::Model(const std::string &primitive_name, const mVector3 &color) : _lines(std::make_shared<Lines>()) { load(std::make_shared<UniversalMesh>(primitive_name, color)); }
Kasumi::Model::Model(std::vector<UniversalMesh::Vertex> &&vertices, std::vector<Index> &&indices, std::map<std::string, std::vector<TexturePtr>> &&textures) : _lines(std::make_shared<Lines>()) { load(std::make_shared<UniversalMesh>(std::move(vertices), std::move(indices), std::move(textures))); }
Kasumi::Model::~Model() { std::cout << "delete model: " << _path << std::endl; }

// ================================================== Public Methods ==================================================

Kasumi::ShaderPtr Kasumi::Model::_default_mesh_shader = nullptr;
Kasumi::ShaderPtr Kasumi::Model::_default_instanced_mesh_shader = nullptr;
Kasumi::ShaderPtr Kasumi::Model::_default_line_shader = nullptr;
void Kasumi::Model::update_mvp(const mMatrix4x4 &model, const mMatrix4x4 &view, const mMatrix4x4 &projection)
{
	_shader->use();
	_shader->uniform("model", model);
	_shader->uniform("view", view);
	_shader->uniform("projection", projection);

	if (_opt.render_bbox)
	{
		_default_line_shader->use();
		_default_line_shader->uniform("model", model);
		_default_line_shader->uniform("view", view);
		_default_line_shader->uniform("projection", projection);
	}

	if (_opt.instancing)
	{
		if (_opt.instance_dirty && !_instance_matrices.empty())
		{
			glBindBuffer(GL_ARRAY_BUFFER, _opt.instanceVBO);
			glBufferData(GL_ARRAY_BUFFER, _instance_matrices.size() * sizeof(mMatrix4x4), &_instance_matrices[0], GL_DYNAMIC_DRAW);
			for (auto &&mesh: _meshes)
				mesh->_opt.instance_count = _instance_matrices.size();
		}
	}
}
void Kasumi::Model::render()
{
	_shader->use();

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
		for (auto &mesh: _meshes)
			mesh->render(_shader);

	if (_opt.render_bbox)
	{
		_default_line_shader->use();
		for (auto &line: _lines->lines())
			line.color = _opt.bbox_color;
		_lines->render(_default_line_shader);
	}
}
auto Kasumi::Model::vertices(size_t i) -> std::vector<Vertex> &
{
	_meshes[i]->mark_dirty();
	return _meshes[i]->_verts;
}
auto Kasumi::Model::mesh_size() const -> size_t { return _meshes.size(); }
void Kasumi::Model::instancing()
{
	_shader = _default_instanced_mesh_shader;

	_opt.instancing = true;

	glGenBuffers(1, &_opt.instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _opt.instanceVBO);

	for (auto &&mesh: _meshes)
	{
		glBindVertexArray(mesh->_vao);
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(mMatrix4x4), (void *) 0);
		glEnableVertexAttribArray(8);
		glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(mMatrix4x4), (void *) (sizeof(mVector4)));
		glEnableVertexAttribArray(9);
		glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(mMatrix4x4), (void *) (2 * sizeof(mVector4)));
		glEnableVertexAttribArray(10);
		glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, sizeof(mMatrix4x4), (void *) (3 * sizeof(mVector4)));

		glVertexAttribDivisor(7, 1);
		glVertexAttribDivisor(8, 1);
		glVertexAttribDivisor(9, 1);
		glVertexAttribDivisor(10, 1);

		glBindVertexArray(0);

		mesh->_opt.instanced = true;
	}

	_instance_matrices.reserve(100000); // prepare a large capacity vector, to promote performance.
}
void Kasumi::Model::add_instances(const std::vector<Pose> &poses)
{
	for (auto &&pose: poses) _instance_matrices.push_back(pose.get_model_matrix().transposed());
	_opt.instance_dirty = true;
}
void Kasumi::Model::add_instances(const Kasumi::Pose &pose)
{
	_instance_matrices.push_back(pose.get_model_matrix().transposed());
	_opt.instance_dirty = true;
}
void Kasumi::Model::clear_instances()
{
	_instance_matrices.clear();
	_opt.instance_dirty = true;
}
// ================================================== Public Methods ==================================================



// ================================================== Private Methods ==================================================

auto Kasumi::Model::load(const std::string &path) -> bool
{
	if (_default_mesh_shader == nullptr)
		_default_mesh_shader = std::make_shared<Shader>(std::string(BuiltinShaderDir) + "default_shader_vertex.glsl", std::string(BuiltinShaderDir) + "default_shader_fragment.glsl");
	if (_default_instanced_mesh_shader == nullptr)
		_default_instanced_mesh_shader = std::make_shared<Shader>(std::string(BuiltinShaderDir) + "default_instanced_shader_vertex.glsl", std::string(BuiltinShaderDir) + "default_shader_fragment.glsl");
	if (_default_line_shader == nullptr)
		_default_line_shader = std::make_shared<Shader>(std::string(BuiltinShaderDir) + "default_line_shader_vertex.glsl", std::string(BuiltinShaderDir) + "default_line_shader_fragment.glsl");
	_shader = _default_mesh_shader;

	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		return false;

	process_node(scene->mRootNode, scene);

	for (auto &m: _meshes)
		_bbox.merge(m->_bbox);

	auto l = _bbox.lowerCorner;
	auto u = _bbox.upperCorner;
	_lines->add(mVector3(l.x, l.y, l.z), mVector3(u.x, l.y, l.z));
	_lines->add(mVector3(l.x, l.y, l.z), mVector3(l.x, u.y, l.z));
	_lines->add(mVector3(l.x, l.y, l.z), mVector3(l.x, l.y, u.z));
	_lines->add(mVector3(u.x, u.y, u.z), mVector3(l.x, u.y, u.z));
	_lines->add(mVector3(u.x, u.y, u.z), mVector3(u.x, l.y, u.z));
	_lines->add(mVector3(u.x, u.y, u.z), mVector3(u.x, u.y, l.z));
	_lines->add(mVector3(l.x, u.y, u.z), mVector3(l.x, u.y, l.z));
	_lines->add(mVector3(l.x, u.y, u.z), mVector3(l.x, l.y, u.z));
	_lines->add(mVector3(u.x, l.y, u.z), mVector3(u.x, l.y, l.z));
	_lines->add(mVector3(u.x, l.y, u.z), mVector3(u.x, u.y, u.z));
	_lines->add(mVector3(u.x, l.y, l.z), mVector3(u.x, u.y, l.z));
	_lines->add(mVector3(u.x, l.y, l.z), mVector3(l.x, l.y, l.z));
	return true;
}
auto Kasumi::Model::load(Kasumi::UniversalMeshPtr &&mesh) -> bool
{
	if (_default_mesh_shader == nullptr)
		_default_mesh_shader = std::make_shared<Shader>(std::string(BuiltinShaderDir) + "default_shader_vertex.glsl", std::string(BuiltinShaderDir) + "default_shader_fragment.glsl");
	if (_default_instanced_mesh_shader == nullptr)
		_default_instanced_mesh_shader = std::make_shared<Shader>(std::string(BuiltinShaderDir) + "default_instanced_shader_vertex.glsl", std::string(BuiltinShaderDir) + "default_shader_fragment.glsl");
	if (_default_line_shader == nullptr)
		_default_line_shader = std::make_shared<Shader>(std::string(BuiltinShaderDir) + "default_line_shader_vertex.glsl", std::string(BuiltinShaderDir) + "default_line_shader_fragment.glsl");
	_shader = _default_mesh_shader;

	_meshes.emplace_back(std::move(mesh));

	for (auto &m: _meshes)
		_bbox.merge(m->_bbox);

	auto l = _bbox.lowerCorner;
	auto u = _bbox.upperCorner;

	// cube's all edges
	_lines->add(mVector3(l.x, l.y, l.z), mVector3(u.x, l.y, l.z));
	_lines->add(mVector3(u.x, l.y, l.z), mVector3(u.x, u.y, l.z));
	_lines->add(mVector3(u.x, u.y, l.z), mVector3(l.x, u.y, l.z));
	_lines->add(mVector3(l.x, u.y, l.z), mVector3(l.x, l.y, l.z));
	_lines->add(mVector3(l.x, l.y, u.z), mVector3(u.x, l.y, u.z));
	_lines->add(mVector3(u.x, l.y, u.z), mVector3(u.x, u.y, u.z));
	_lines->add(mVector3(u.x, u.y, u.z), mVector3(l.x, u.y, u.z));
	_lines->add(mVector3(l.x, u.y, u.z), mVector3(l.x, l.y, u.z));
	_lines->add(mVector3(l.x, l.y, l.z), mVector3(l.x, l.y, u.z));
	_lines->add(mVector3(u.x, l.y, l.z), mVector3(u.x, l.y, u.z));
	_lines->add(mVector3(u.x, u.y, l.z), mVector3(u.x, u.y, u.z));
	_lines->add(mVector3(l.x, u.y, l.z), mVector3(l.x, u.y, u.z));


	return true;
}
void Kasumi::Model::process_node(const struct aiNode *node, const struct aiScene *scene)
{
	for (int i = 0; i < node->mNumMeshes; ++i)
	{
		auto *mesh = scene->mMeshes[node->mMeshes[i]];
		_meshes.emplace_back(std::move(process_mesh(mesh, scene)));
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++)
		process_node(node->mChildren[i], scene);
}
auto Kasumi::Model::process_mesh(const aiMesh *mesh, const aiScene *scene) -> Kasumi::UniversalMeshPtr
{
	std::vector<Kasumi::Model::Vertex> vertices;
	std::vector<Kasumi::Model::Index> indices;

	// Load vertices
	for (int i = 0; i < mesh->mNumVertices; ++i)
	{
		Kasumi::Model::Vertex v;
		v.position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
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

	// Load indices
	for (int i = 0; i < mesh->mNumFaces; ++i)
		for (int j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
			indices.emplace_back(mesh->mFaces[i].mIndices[j]);

	// Load materials
	auto *materials = scene->mMaterials[mesh->mMaterialIndex];
	auto load_material = [&](aiTextureType type) -> std::vector<Kasumi::TexturePtr>
	{
		std::vector<Kasumi::TexturePtr> res;
		for (int i = 0; i < materials->GetTextureCount(type); ++i)
		{
			aiString aiPath;
			materials->GetTexture(type, i, &aiPath);
			std::string tex_path(aiPath.C_Str());
			std::replace(tex_path.begin(), tex_path.end(), '\\', '/');
			std::string absolute_path = _path.substr(0, _path.find_last_of('/')) + "/" + tex_path;
			res.push_back(std::move(std::make_shared<Kasumi::Texture>(absolute_path)));

			std::cout << absolute_path << std::endl;
		}
		return res;
	};

	std::map<std::string, std::vector<Kasumi::TexturePtr>> textures;
	textures["diffuse"] = load_material(aiTextureType_DIFFUSE);
	textures["specular"] = load_material(aiTextureType_SPECULAR);
	textures["normal"] = load_material(aiTextureType_NORMALS);
	textures["height"] = load_material(aiTextureType_HEIGHT);
	textures["ambient"] = load_material(aiTextureType_AMBIENT);

	return std::make_shared<Kasumi::UniversalMesh>(std::move(vertices), std::move(indices), std::move(textures));
}
auto Kasumi::Model::center_of_gravity() const -> mVector3
{
	mVector3 res;
	for (auto &&mesh: _meshes)
		res += mesh->_center_point;
	return res / static_cast<float>(_meshes.size());
}

// ================================================== Private Methods ==================================================