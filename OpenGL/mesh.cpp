#include "../mesh.h"

#include "glad/glad.h"

Kasumi::TexturedMesh::TexturedMesh(std::vector<Vertex> &&vertices, std::vector<Index> &&indices, std::map<std::string, TexturePtr> &&diffuse_textures, std::map<std::string, TexturePtr> &&specular_textures, std::map<std::string, TexturePtr> &&normal_textures,
                                   std::map<std::string, TexturePtr> &&height_textures) : _verts(std::move(vertices)), _idxs(std::move(indices)), _diffuse_textures(std::move(diffuse_textures)), _specular_textures(std::move(specular_textures)),
                                                                                          _normal_textures(std::move(normal_textures)), _height_textures(std::move(height_textures)), _shader(nullptr)
{
    VAO = VBO = EBO = 0;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *) 0); // location = 0, position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *) sizeof(mVector3)); // location = 1, normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *) (2 * sizeof(mVector3))); // location = 2, tex_coord
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *) (2 * sizeof(mVector3) + sizeof(mVector2))); // location = 3, tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *) (3 * sizeof(mVector3) + sizeof(mVector2))); // location = 4, bi_tangent
    glEnableVertexAttribArray(4);
    glVertexAttribIPointer(5, 1, GL_UNSIGNED_INT, sizeof(Vertex), (GLvoid *) (4 * sizeof(mVector3) + sizeof(mVector2))); // location = 5, id
    glEnableVertexAttribArray(5);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    glBindVertexArray(0);

    _center_point = mVector3(0.0f, 0.0f, 0.0f);
    for (auto &v: _verts)
        _center_point += v.position;
    _center_point /= _verts.size();

    n_elem = _idxs.size();
    _bbox.reset();
    for (auto &v: _verts)
        _bbox.merge(v.position);
    dirty = true;
    is_inited = true;
}

Kasumi::TexturedMesh::~TexturedMesh()
{
    glDeleteBuffers(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &EBO);
    VAO = VBO = EBO = 0;

    std::cout << "DELETE TEXTURED MESH" << std::endl;
}

void Kasumi::TexturedMesh::render()
{
    if (dirty)
        update();

    _shader->use();
    _shader->uniform("has_color", false);
    if (!_diffuse_textures.empty())
    {
        _diffuse_textures.begin()->second->bind(0);
        _shader->uniform("texture_diffuse", 0);
        _shader->uniform("has_diffuse_texture", true);
    } else
        _shader->uniform("has_diffuse_texture", false);
    if (!_specular_textures.empty())
    {
        _specular_textures.begin()->second->bind(1);
        _shader->uniform("texture_specular", 1);
        _shader->uniform("has_specular_texture", true);
    } else
        _shader->uniform("has_specular_texture", false);
    if (!_normal_textures.empty())
    {
        _normal_textures.begin()->second->bind(2);
        _shader->uniform("texture_normal", 2);
        _shader->uniform("has_normal_texture", true);
    } else
        _shader->uniform("has_normal_texture", false);
    if (!_height_textures.empty())
    {
        _height_textures.begin()->second->bind(3);
        _shader->uniform("texture_height", 3);
        _shader->uniform("has_height_texture", true);
    } else
        _shader->uniform("has_height_texture", false);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, (GLuint) n_elem, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}
void Kasumi::TexturedMesh::update()
{
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * _verts.size(), &_verts[0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index) * _idxs.size(), &_idxs[0], GL_DYNAMIC_DRAW);

    glBindVertexArray(0);

    dirty = false;
}
void Kasumi::TexturedMesh::use_shader(const Kasumi::ShaderPtr &shader) { _shader = shader; }
auto Kasumi::TexturedMesh::get_shader() -> Kasumi::ShaderPtr & { return _shader; }
auto Kasumi::TexturedMesh::get_center_point() const -> Kasumi::mVector3 { return _center_point; }

void Kasumi::TexturedMesh::print_info() const
{
    std::cout << "| Vertices count: " << _verts.size() << " |" << std::endl;
    std::cout << "| Indices count: " << _idxs.size() << " |" << std::endl;
    if (!_diffuse_textures.empty())
    {
        std::cout << "| Diffuse textures count: " << _diffuse_textures.size() << " |" << std::endl;
        for (auto &e: _diffuse_textures)
            e.second->print_info();
    }
    if (!_specular_textures.empty())
    {
        std::cout << "| Specular textures count: " << _specular_textures.size() << " |" << std::endl;
        for (auto &e: _specular_textures)
            e.second->print_info();
    }
    if (!_normal_textures.empty())
    {
        std::cout << "| Normal textures count: " << _normal_textures.size() << " |" << std::endl;
        for (auto &e: _normal_textures)
            e.second->print_info();
    }
    if (!_height_textures.empty())
    {
        std::cout << "| Height textures count: " << _height_textures.size() << " |" << std::endl;
        for (auto &e: _height_textures)
            e.second->print_info();
    }
}


Kasumi::ColoredMesh::ColoredMesh(std::vector<Vertex> &&vertices, std::vector<Index> &&indices) {}
Kasumi::ColoredMesh::~ColoredMesh() {}
void Kasumi::ColoredMesh::render() {}
void Kasumi::ColoredMesh::use_shader(const Kasumi::ShaderPtr &shader) { _shader = shader; }
auto Kasumi::ColoredMesh::get_shader() -> Kasumi::ShaderPtr & { return _shader; }
auto Kasumi::ColoredMesh::get_center_point() const -> Kasumi::mVector3 { return Kasumi::mVector3(); }
