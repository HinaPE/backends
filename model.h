#ifndef KASUMI_MODEL_H
#define KASUMI_MODEL_H

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "texture.h"
#include "mesh.h"
#include "shader.h"
#include "pose.h"

#include <vector>
#include <string>
#include <memory>

namespace Kasumi
{
class Model
{
public: //! ==================== Model Info ====================
	using Vertex = UniversalMesh::Vertex;
	using Index = UniversalMesh::Index;
	auto vertices(size_t i) -> std::vector<Vertex> &;
private:
	std::vector<UniversalMeshPtr> _meshes;
	std::string _path;

public: //! ==================== Rendering Options ====================
	struct Opt
	{
		// rendering options
		bool depth_test = true;
		bool render_wireframe = false;

		// instancing
		bool instancing = false;
		unsigned int instanceVBO;
		int instance_count;
		std::vector<mMatrix4x4> instance_matrices;
	} _opt;
	inline void use_custom_shader(const ShaderPtr &shader) { _shader = shader; }
	void setup_instancing(const std::vector<Pose> &instance_poses);
	void update_mvp(const mMatrix4x4 &model, const mMatrix4x4 &view, const mMatrix4x4 &projection);
	void render();
private:
	static ShaderPtr _default_mesh_shader;
	static ShaderPtr _default_instanced_mesh_shader;
	ShaderPtr _shader;

public: //! ==================== Geometry Info ====================
	auto center_of_gravity() const -> mVector3;
private:
	mVector3 _center_point;

//! ==================== Constructors & Destructor ====================
//! - [DELETE] copy constructor & copy assignment operator
//! - [ENABLE] move constructor & move assignment operator
public:
	Model(const std::string &model_path);
	Model(const std::string &primitive_name, const std::string &texture_name);
	Model(const std::string &primitive_name, const mVector3 &color);
	Model(std::vector<UniversalMesh::Vertex> &&vertices, std::vector<Index> &&indices, std::map<std::string, std::vector<TexturePtr>> &&textures = {});

public:
	Model(const Model &) = delete;
	Model(Model &&) = default;
	~Model();
	auto operator=(const Model &) -> Model & = delete;
	auto operator=(Model &&) -> Model & = default;

private:
	auto load(const std::string &path) -> bool;
	auto load(UniversalMeshPtr &&mesh) -> bool;
	void process_node(const aiNode *node, const aiScene *scene);
	auto process_mesh(const aiMesh *mesh, const aiScene *scene) -> UniversalMeshPtr;
};
using ModelPtr = std::shared_ptr<Model>;
}

#endif //KASUMI_MODEL_H
