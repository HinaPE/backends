#ifndef BACKENDS_MODEL_H
#define BACKENDS_MODEL_H

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "mesh.h"

namespace Kasumi
{
class Model final : public HinaPE::CopyDisable
{
public:
	void render(); // render with built-in shader
	void render(const Shader &shader); // render with custom shader
	explicit Model(const std::string &path, real scale = 1);
	~Model();

private:
	auto _load(const std::string &path) -> bool;
	void _process_node(const aiNode *node, const aiScene *scene);
	auto _process_mesh(const aiMesh *mesh, const aiScene *scene) -> MeshPtr;

private:
	std::vector<MeshPtr> _meshes;
	std::shared_ptr<Lines> _bbox_lines;
	mBBox3 _bbox;
	std::string _path;
	real _scale;
};
using ModelPtr = std::shared_ptr<Model>;
} // namespace Kasumi

#endif //BACKENDS_MODEL_H
