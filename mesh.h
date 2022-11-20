#ifndef KASUMI_MESH_H
#define KASUMI_MESH_H

#include "math_api.h"
#include "shader.h"
#include "texture.h"

#include <map>

namespace Kasumi
{
class TexturedMesh final
{
public:
    struct Vertex
    {
        mVector3 position;
        mVector3 normal;
        mVector2 tex_coord;
        mVector3 tangent;
        mVector3 bi_tangent;
        unsigned int id;
    };
    using Index = unsigned int;
    void print_info() const;

public:
    TexturedMesh() = default;
    TexturedMesh(std::vector<Vertex> &&vertices, std::vector<Index> &&indices, std::map<std::string, TexturePtr> &&diffuse_textures = {}, std::map<std::string, TexturePtr> &&specular_textures = {}, std::map<std::string, TexturePtr> &&normal_textures = {},
                 std::map<std::string, TexturePtr> &&height_textures = {});
    TexturedMesh(const TexturedMesh &src) = delete;
    TexturedMesh(TexturedMesh &&src) noexcept = default;
    ~TexturedMesh();
    void operator=(const TexturedMesh &src) = delete;
    auto operator=(TexturedMesh &&src) noexcept -> TexturedMesh & = default;

public:
    void render();
    void use_shader(const ShaderPtr &shader);
    auto get_shader() -> ShaderPtr &;
    auto get_center_point() const -> mVector3;

private:
    void init(std::vector<Vertex> &&vertices, std::vector<Index> &&indices, std::map<std::string, TexturePtr> &&diffuse_textures = {}, std::map<std::string, TexturePtr> &&specular_textures = {}, std::map<std::string, TexturePtr> &&normal_textures = {},
              std::map<std::string, TexturePtr> &&height_textures = {});
    void update();

private:
    bool is_inited;
    bool dirty;
    unsigned int VAO, VBO, EBO;
    size_t n_elem;

    std::vector<Vertex> _verts;
    std::vector<Index> _idxs;
    std::map<std::string, TexturePtr> _diffuse_textures;
    std::map<std::string, TexturePtr> _specular_textures;
    std::map<std::string, TexturePtr> _normal_textures;
    std::map<std::string, TexturePtr> _height_textures;
    mVector3 _center_point;
    mBBox _bbox;
    ShaderPtr _shader;
};
using TexturedMeshPtr = std::shared_ptr<TexturedMesh>;

class ColoredMesh final {
public:
    struct Vertex
    {
        mVector3 position;
        mVector3 normal;
        mVector3 color;
        mVector3 tangent;
        mVector3 bi_tangent;
        unsigned int id;
    };
    using Index = unsigned int;

public:
    ColoredMesh() = default;
    ColoredMesh(const std::string& primitive_name, const std::string& color_name);
    ColoredMesh(std::vector<Vertex> &&vertices, std::vector<Index> &&indices, const std::string& color_name);
    ColoredMesh(const ColoredMesh &src) = delete;
    ColoredMesh(ColoredMesh &&src) noexcept = default;
    ~ColoredMesh();
    void operator=(const ColoredMesh &src) = delete;
    auto operator=(ColoredMesh &&src) noexcept -> ColoredMesh & = default;

public:
    void render();
    void use_shader(const ShaderPtr &shader);
    auto get_shader() -> ShaderPtr &;
    auto get_center_point() const -> mVector3;

private:
    void init(std::vector<Vertex> &&vertices, std::vector<Index> &&indices, const std::string& color_name);
    void update();

private:
    bool is_inited;
    bool dirty;
    unsigned int VAO, VBO, EBO;
    size_t n_elem;

    std::vector<Vertex> _verts;
    std::vector<Index> _idxs;
    mVector3 _center_point;
    mBBox _bbox;
    ShaderPtr _shader;
};
using ColoredMeshPtr = std::shared_ptr<ColoredMesh>;
}

#endif //KASUMI_MESH_H
