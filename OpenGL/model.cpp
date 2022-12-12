#include "glad/glad.h"
#include "../model.h"

#include <map>
#include <utility>
#include <algorithm>

Kasumi::Model::Model(const std::string &model_path) : _path(model_path) { load(model_path); }
Kasumi::Model::Model(const std::string &primitive_name, const std::string &texture_name) { load(std::make_shared<UniversalMesh>(primitive_name, texture_name)); }
Kasumi::Model::Model(const std::string &primitive_name, const mVector3 &color) { load(std::make_shared<UniversalMesh>(primitive_name, color)); }
Kasumi::Model::Model(std::vector<UniversalMesh::Vertex> &&vertices, std::vector<Index> &&indices, std::map<std::string, std::vector<TexturePtr>> &&textures) { load(std::make_shared<UniversalMesh>(std::move(vertices), std::move(indices), std::move(textures))); }
Kasumi::Model::~Model() { std::cout << "delete model: " << _path << std::endl; }

// ================================================== Public Methods ==================================================

Kasumi::ShaderPtr Kasumi::Model::_default_mesh_shader = nullptr;
Kasumi::ShaderPtr Kasumi::Model::_default_instanced_mesh_shader = nullptr;
void Kasumi::Model::update_mvp(const mMatrix4x4 &model, const mMatrix4x4 &view, const mMatrix4x4 &projection)
{
	_shader->use();
	_shader->uniform("model", model);
	_shader->uniform("view", view);
	_shader->uniform("projection", projection);

	if (_opt.instancing)
	{
		glBindBuffer(GL_ARRAY_BUFFER, _opt.instanceVBO);
		glBufferData(GL_ARRAY_BUFFER, _opt.instance_count * sizeof(mMatrix4x4), &_opt.instance_matrices[0], GL_DYNAMIC_DRAW);
	}
}
void Kasumi::Model::render()
{
	_shader->use();

	_opt.depth_test ? glEnable(GL_DEPTH_TEST)
					: glDisable(GL_DEPTH_TEST);
	_opt.render_wireframe ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)
						  : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	for (auto &mesh: _meshes)
		mesh->render(_shader);
}
auto Kasumi::Model::vertices(size_t i) -> std::vector<Vertex> &
{
	_meshes[i]->mark_dirty();
	return _meshes[i]->_verts;
}

// ================================================== Public Methods ==================================================



// ================================================== Private Methods ==================================================

auto Kasumi::Model::load(const std::string &path) -> bool
{
	if (_default_mesh_shader == nullptr)
		_default_mesh_shader = std::make_shared<Shader>(std::string(BuiltinShaderDir) + "default_shader_vertex.glsl", std::string(BuiltinShaderDir) + "default_shader_fragment.glsl");
	if (_default_instanced_mesh_shader == nullptr)
		_default_instanced_mesh_shader = std::make_shared<Shader>(std::string(BuiltinShaderDir) + "default_instanced_shader_vertex.glsl", std::string(BuiltinShaderDir) + "default_shader_fragment.glsl");
	_shader = _default_mesh_shader;

	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		return false;

	process_node(scene->mRootNode, scene);
	return true;
}
auto Kasumi::Model::load(Kasumi::UniversalMeshPtr &&mesh) -> bool
{
	if (_default_mesh_shader == nullptr)
		_default_mesh_shader = std::make_shared<Shader>(std::string(BuiltinShaderDir) + "default_shader_vertex.glsl", std::string(BuiltinShaderDir) + "default_shader_fragment.glsl");
	if (_default_instanced_mesh_shader == nullptr)
		_default_instanced_mesh_shader = std::make_shared<Shader>(std::string(BuiltinShaderDir) + "default_instanced_shader_vertex.glsl", std::string(BuiltinShaderDir) + "default_shader_fragment.glsl");
	_shader = _default_mesh_shader;

	_meshes.emplace_back(std::move(mesh));
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
void Kasumi::Model::setup_instancing(const std::vector<Kasumi::Pose> &instance_poses)
{
	_shader = _default_instanced_mesh_shader;

	_opt.instancing = true;
	_opt.instance_count = instance_poses.size();
	for (auto &pose: instance_poses)
		_opt.instance_matrices.emplace_back(pose.get_model_matrix().transposed());

	glGenBuffers(1, &_opt.instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _opt.instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, _opt.instance_count * sizeof(mMatrix4x4), &_opt.instance_matrices[0], GL_DYNAMIC_DRAW);

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
		mesh->_opt.instance_count = _opt.instance_count;
	}
}
auto Kasumi::Model::center_of_gravity() const -> mVector3
{
	mVector3 res;
	for (auto &&mesh: _meshes)
		res += mesh->_center_point;
	return res / static_cast<float>(_meshes.size());
}

// ================================================== Private Methods ==================================================