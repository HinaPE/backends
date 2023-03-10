#include <utility>

#include "glad/glad.h"
#include "../model.h"
#include "../camera.h"
#include "../light.h"

//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Kasumi::Model::Model(const std::string &model_path, real scale) : _path(model_path), _scale(scale), _bbox_lines(std::make_shared<Lines>()) { _load(model_path); }
Kasumi::Model::~Model() { std::cout << "delete model: " << _path << std::endl; }

void Kasumi::Model::render()
{
	Shader::DefaultMeshShader->use();
	Shader::DefaultMeshShader->uniform("model", mMatrix4x4::Identity());
	Shader::DefaultMeshShader->uniform("view", Camera::MainCamera->get_view());
	Shader::DefaultMeshShader->uniform("projection", Camera::MainCamera->get_projection());
	Shader::DefaultMeshShader->uniform("lightPos", Light::MainLight->_opt.light_pos);
	Shader::DefaultMeshShader->uniform("viewPos", Light::MainLight->_opt.view_pos);

	for (auto &mesh: _meshes)
		mesh->render(*Shader::DefaultMeshShader);

	Shader::DefaultLineShader->use();
	Shader::DefaultLineShader->uniform("model", mMatrix4x4::Identity());
	Shader::DefaultLineShader->uniform("view", Camera::MainCamera->get_view());
	Shader::DefaultLineShader->uniform("projection", Camera::MainCamera->get_projection());
	Shader::DefaultLineShader->uniform("lightPos", Light::MainLight->_opt.light_pos);
	Shader::DefaultLineShader->uniform("viewPos", Light::MainLight->_opt.view_pos);

	_bbox_lines->render(*Shader::DefaultLineShader);
}

void Kasumi::Model::render(const Shader &shader)
{
	shader.use();
	shader.uniform("view", Camera::MainCamera->get_view());
	shader.uniform("projection", Camera::MainCamera->get_projection());
	shader.uniform("lightPos", Light::MainLight->_opt.light_pos);
	shader.uniform("viewPos", Light::MainLight->_opt.view_pos);

	for (auto &mesh: _meshes)
		mesh->render(shader);

	Shader::DefaultLineShader->use();
	Shader::DefaultLineShader->uniform("view", Camera::MainCamera->get_view());
	Shader::DefaultLineShader->uniform("projection", Camera::MainCamera->get_projection());
	Shader::DefaultLineShader->uniform("lightPos", Light::MainLight->_opt.light_pos);
	Shader::DefaultLineShader->uniform("viewPos", Light::MainLight->_opt.view_pos);

	_bbox_lines->render(*Shader::DefaultLineShader);
}

auto Kasumi::Model::_load(const std::string &path) -> bool
{
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		return false;

	_process_node(scene->mRootNode, scene);

	if (_scale != 1)
		for (auto &m: _meshes)
			for (auto &v: m->vertices())
				v.position *= _scale;

	for (auto &m: _meshes)
		_bbox.merge(m->_bbox);

	auto l = _bbox._lower_corner;
	auto u = _bbox._upper_corner;
	_bbox_lines->add(mVector3(l.x(), l.y(), l.z()), mVector3(u.x(), l.y(), l.z()));
	_bbox_lines->add(mVector3(l.x(), l.y(), l.z()), mVector3(l.x(), u.y(), l.z()));
	_bbox_lines->add(mVector3(l.x(), l.y(), l.z()), mVector3(l.x(), l.y(), u.z()));
	_bbox_lines->add(mVector3(u.x(), u.y(), u.z()), mVector3(l.x(), u.y(), u.z()));
	_bbox_lines->add(mVector3(u.x(), u.y(), u.z()), mVector3(u.x(), l.y(), u.z()));
	_bbox_lines->add(mVector3(u.x(), u.y(), u.z()), mVector3(u.x(), u.y(), l.z()));
	_bbox_lines->add(mVector3(l.x(), u.y(), u.z()), mVector3(l.x(), u.y(), l.z()));
	_bbox_lines->add(mVector3(l.x(), u.y(), u.z()), mVector3(l.x(), l.y(), u.z()));
	_bbox_lines->add(mVector3(u.x(), l.y(), u.z()), mVector3(u.x(), l.y(), l.z()));
	_bbox_lines->add(mVector3(u.x(), l.y(), u.z()), mVector3(u.x(), u.y(), u.z()));
	_bbox_lines->add(mVector3(u.x(), l.y(), l.z()), mVector3(u.x(), u.y(), l.z()));
	_bbox_lines->add(mVector3(u.x(), l.y(), l.z()), mVector3(l.x(), l.y(), l.z()));
	return true;
}

void Kasumi::Model::_process_node(const struct aiNode *node, const struct aiScene *scene)
{
	for (int i = 0; i < node->mNumMeshes; ++i)
	{
		auto *mesh = scene->mMeshes[node->mMeshes[i]];
		_meshes.emplace_back(std::move(_process_mesh(mesh, scene)));
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++)
		_process_node(node->mChildren[i], scene);
}

auto Kasumi::Model::_process_mesh(const aiMesh *mesh, const aiScene *scene) -> Kasumi::MeshPtr
{
	std::vector<Mesh::Vertex> vertices;
	std::vector<Mesh::Index> indices;

	// Load vertices
	for (int i = 0; i < mesh->mNumVertices; ++i)
	{
		Mesh::Vertex v;
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

			if (scene->GetEmbeddedTexture(aiPath.C_Str()) != nullptr)
			{
				auto *tex = scene->GetEmbeddedTexture(aiPath.C_Str());

				int width, height, nr_channels;
				unsigned char *data = stbi_load_from_memory(reinterpret_cast<unsigned char *>(tex->pcData), tex->mWidth, &width, &height, &nr_channels, 0);
				if (data)
					res.push_back(std::move(std::make_shared<Kasumi::Texture>(data, width, height, nr_channels)));
				else
					std::cout << "Failed to load embedded texture: " << tex_path << std::endl;
			} else
				res.push_back(std::move(std::make_shared<Kasumi::Texture>(absolute_path)));

			std::cout << "Load texture: " << absolute_path << std::endl;
		}
		return res;
	};

	std::map<std::string, std::vector<Kasumi::TexturePtr>> textures;
	textures["diffuse"] = load_material(aiTextureType_DIFFUSE);
	textures["specular"] = load_material(aiTextureType_SPECULAR);
	textures["normal"] = load_material(aiTextureType_NORMALS);
	textures["height"] = load_material(aiTextureType_HEIGHT);
	textures["ambient"] = load_material(aiTextureType_AMBIENT);

	return std::make_shared<Kasumi::Mesh>(std::move(vertices), std::move(indices), std::move(textures));
}
