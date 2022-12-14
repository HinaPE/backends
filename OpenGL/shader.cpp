#include "../shader.h"
#include "glad/glad.h"
#include <fstream>
#include <exception>

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

	validate(v, "VERTEX");
	validate(f, "FRAGMENT");
	validate(ID, "PROGRAM");

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

	validate(v, "VERTEX");
	validate(f, "FRAGMENT");
	validate(ID, "PROGRAM");

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
void Kasumi::Shader::validate(unsigned int shader, const std::string &type)
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
void Kasumi::Shader::uniform(const std::string &name, bool value) const { glUniform1i(glGetUniformLocation(ID, name.c_str()), static_cast<int>(value)); }
void Kasumi::Shader::uniform(const std::string &name, int value) const { glUniform1i(glGetUniformLocation(ID, name.c_str()), value); }
void Kasumi::Shader::uniform(const std::string &name, unsigned int value) const { glUniform1ui(glGetUniformLocation(ID, name.c_str()), value); }
void Kasumi::Shader::uniform(const std::string &name, float value) const { glUniform1f(glGetUniformLocation(ID, name.c_str()), value); }
void Kasumi::Shader::uniform(const std::string &name, const mVector2 &value) const { glUniform2f(glGetUniformLocation(ID, name.c_str()), value.x, value.y); }
void Kasumi::Shader::uniform(const std::string &name, const mVector3 &value) const { glUniform3f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z); }
void Kasumi::Shader::uniform(const std::string &name, const mVector4 &value) const { glUniform4f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z, value.w); }
void Kasumi::Shader::uniform(const std::string &name, mMatrix3x3 value) const { glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, value.transposed().data()); }
void Kasumi::Shader::uniform(const std::string &name, mMatrix4x4 value) const { glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, value.transposed().data()); }

