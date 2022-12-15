#ifndef KASUMI_MODEL_H
#define KASUMI_MODEL_H

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "light.h"
#include "mesh.h"
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
	auto mesh_size() const -> size_t;
	void instancing();
	void add_instances(const std::vector<Pose>& poses);
	void add_instances(const Pose& pose);
	void clear_instances();
private:
	std::vector<UniversalMeshPtr> _meshes;
	std::vector<mMatrix4x4> _instance_matrices; // make sure instancing() is called before use this
	LinesPtr _lines;
	std::string _path;

public: //! ==================== Rendering Options ====================
	struct Opt
	{
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

		// instancing
		bool instancing = false;
		bool instance_dirty = true;
		unsigned int instanceVBO;
	} _opt;
	inline void use_custom_shader(const ShaderPtr &shader) { _shader = shader; }
	void update_mvp(const mMatrix4x4 &model, const mMatrix4x4 &view, const mMatrix4x4 &projection);
	void update_light(const LightPtr &light);
	void render();
	void framebuffer_mode(bool mode); // when this is on, the model will be rendered transparently
private:
	static ShaderPtr _default_mesh_shader;
	static ShaderPtr _default_instanced_mesh_shader;
	static ShaderPtr _default_line_shader;
	ShaderPtr _shader;

public: //! ==================== Geometry Info ====================
	auto center_of_gravity() const -> mVector3;
private:
	mBBox _bbox;
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
