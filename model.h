#ifndef KASUMI_MODEL_H
#define KASUMI_MODEL_H

#include "texture.h"
#include "mesh.h"
#include "shader.h"

#include <map>
#include <string>
#include <memory>

namespace Kasumi
{
class Model
{
public:
    auto load(const std::string &path) -> bool;
    void render();
    void use_shader(const ShaderPtr &shader);
    auto get_shader() -> ShaderPtr &;
    auto get_center_point() const -> mVector3;

public:
    Model() = default;
    Model(const std::string &path, ShaderPtr shader);
    Model(const Model &) = delete;
    Model(Model &&) = default;
    ~Model();
    auto operator=(const Model &) -> Model & = delete;
    auto operator=(Model &&) -> Model & = default;

public:
    void print_info() const;

private:
    std::string _path;
    std::map<std::string, TexturedMeshPtr> _meshes;
    mVector3 _center_point;
    ShaderPtr _shader;
};
using ModelPtr = std::shared_ptr<Model>;
}

#endif //KASUMI_MODEL_H
