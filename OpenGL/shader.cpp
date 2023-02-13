#include "../shader.h"
#include "glad/glad.h"
#include <fstream>
#include <exception>

std::unique_ptr<Kasumi::Shader> Kasumi::Shader::DefaultMeshShader = nullptr;
std::unique_ptr<Kasumi::Shader> Kasumi::Shader::DefaultInstanceShader = nullptr;
std::unique_ptr<Kasumi::Shader> Kasumi::Shader::DefaultLineShader = nullptr;
std::unique_ptr<Kasumi::Shader> Kasumi::Shader::DefaultPointShader = nullptr;

Kasumi::Shader::Shader(const std::string &vertex_path, const std::string &fragment_path) : Shader(vertex_path, fragment_path, "") {}
Kasumi::Shader::Shader(const std::string &vertex_path, const std::string &fragment_path, const std::string &geometry_path)
{
	std::ifstream vertex_shader_stream(vertex_path);
	std::string vertex_shader_src((std::istreambuf_iterator<char>(vertex_shader_stream)), std::istreambuf_iterator<char>());
	const char *vertex_shader_source = vertex_shader_src.c_str();

	std::ifstream fragment_shader_stream(fragment_path);
	std::string fragment_shader_src((std::istreambuf_iterator<char>(fragment_shader_stream)), std::istreambuf_iterator<char>());
	const char *fragment_shader_source = fragment_shader_src.c_str();

	// TODO: enable geometry shader
	if (!geometry_path.empty())
	{
		std::ifstream geometry_shader_stream(geometry_path);
		std::string geometry_shader_src((std::istreambuf_iterator<char>(geometry_shader_stream)), std::istreambuf_iterator<char>());
		const char *geometry_shader_source = geometry_shader_src.c_str();
	}

	auto v = glCreateShader(GL_VERTEX_SHADER);
	auto f = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(v, 1, &vertex_shader_source, nullptr);
	glShaderSource(f, 1, &fragment_shader_source, nullptr);
	glCompileShader(v);
	glCompileShader(f);

	ID = glCreateProgram();
	glAttachShader(ID, v);
	glAttachShader(ID, f);
	glLinkProgram(ID);

	_validate(v, "VERTEX");
	_validate(f, "FRAGMENT");
	_validate(ID, "PROGRAM");

	// to save GPU memory
	glDeleteShader(v);
	glDeleteShader(f);
}
Kasumi::Shader::Shader(const char *vertex_src, const char *fragment_src) : Shader(vertex_src, fragment_src, nullptr) {}
Kasumi::Shader::Shader(const char *vertex_src, const char *fragment_src, const char *geometry_src)
{
	auto v = glCreateShader(GL_VERTEX_SHADER);
	auto f = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(v, 1, &vertex_src, nullptr);
	glShaderSource(f, 1, &fragment_src, nullptr);
	glCompileShader(v);
	glCompileShader(f);

	ID = glCreateProgram();
	glAttachShader(ID, v);
	glAttachShader(ID, f);
	glLinkProgram(ID);

	_validate(v, "VERTEX");
	_validate(f, "FRAGMENT");
	_validate(ID, "PROGRAM");

	// to save GPU memory
	glDeleteShader(v);
	glDeleteShader(f);
}
#include <iostream>
Kasumi::Shader::~Shader()
{
	glUseProgram(0);
	glDeleteProgram(ID);

	std::cout << "delete shader " << std::endl;
}
void Kasumi::Shader::use() const
{
	glUseProgram(ID);
}
void Kasumi::Shader::Init()
{
	if (DefaultMeshShader == nullptr)
		DefaultMeshShader = std::make_unique<Shader>(std::string(BuiltinShaderDir) + "default_shader_vertex.glsl", std::string(BuiltinShaderDir) + "default_shader_fragment.glsl");
	if (DefaultInstanceShader == nullptr)
		DefaultInstanceShader = std::make_unique<Shader>(std::string(BuiltinShaderDir) + "default_instanced_shader_vertex.glsl", std::string(BuiltinShaderDir) + "default_shader_fragment.glsl");
	if (DefaultLineShader == nullptr)
		DefaultLineShader = std::make_unique<Shader>(std::string(BuiltinShaderDir) + "default_line_shader_vertex.glsl", std::string(BuiltinShaderDir) + "default_line_shader_fragment.glsl");
//		if (DefaultPointShader == nullptr)
//			DefaultPointShader = std::make_unique<Shader>(std::string(BuiltinShaderDir) + "default_point_shader_vertex.glsl", std::string(BuiltinShaderDir) + "default_point_shader_fragment.glsl");
}
void Kasumi::Shader::_validate(unsigned int shader, const std::string &type)
{
	int success;
	char infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			throw std::runtime_error("shader compile error");
		}
	} else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			throw std::runtime_error("shader compile error");
		}
	}
}

#ifdef HINAPE_DOUBLE
#define HINA_GL_1F glUniform1d
#define HINA_GL_2F glUniform2d
#define HINA_GL_3F glUniform3d
#define HINA_GL_4F glUniform4d
#define HINA_GL_M3 glUniformMatrix3dv
#define HINA_GL_M4 glUniformMatrix4dv
#else
#define HINA_GL_1F glUniform1f
#define HINA_GL_2F glUniform2f
#define HINA_GL_3F glUniform3f
#define HINA_GL_4F glUniform4f
#define HINA_GL_M3 glUniformMatrix3fv
#define HINA_GL_M4 glUniformMatrix4fv
#endif

void Kasumi::Shader::uniform(const std::string &name, bool value) const { glUniform1i(glGetUniformLocation(ID, name.c_str()), static_cast<int>(value)); }
void Kasumi::Shader::uniform(const std::string &name, int value) const { glUniform1i(glGetUniformLocation(ID, name.c_str()), value); }
void Kasumi::Shader::uniform(const std::string &name, unsigned int value) const { glUniform1ui(glGetUniformLocation(ID, name.c_str()), value); }
void Kasumi::Shader::uniform(const std::string &name, float value) const { HINA_GL_1F(glGetUniformLocation(ID, name.c_str()), value); }
void Kasumi::Shader::uniform(const std::string &name, const mVector2 &value) const { HINA_GL_2F(glGetUniformLocation(ID, name.c_str()), value.x(), value.y()); }
void Kasumi::Shader::uniform(const std::string &name, const mVector3 &value) const { HINA_GL_3F(glGetUniformLocation(ID, name.c_str()), value.x(), value.y(), value.z()); }
void Kasumi::Shader::uniform(const std::string &name, const mVector4 &value) const { HINA_GL_4F(glGetUniformLocation(ID, name.c_str()), value.x(), value.y(), value.z(), value.w()); }
void Kasumi::Shader::uniform(const std::string &name, const mMatrix3x3 &value) const { HINA_GL_M3(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, value.data()); }
void Kasumi::Shader::uniform(const std::string &name, const mMatrix4x4 &value) const { HINA_GL_M4(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, value.data()); }
