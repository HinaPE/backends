#include "../shader.h"
#include "glad/glad.h"
#include <fstream>
#include <exception>

std::shared_ptr<Kasumi::Shader> Kasumi::Shader::DefaultMeshShader = nullptr;
std::shared_ptr<Kasumi::Shader> Kasumi::Shader::DefaultInstanceShader = nullptr;
std::shared_ptr<Kasumi::Shader> Kasumi::Shader::DefaultLineShader = nullptr;
std::shared_ptr<Kasumi::Shader> Kasumi::Shader::DefaultInstanceLineShader = nullptr;
std::shared_ptr<Kasumi::Shader> Kasumi::Shader::DefaultPointShader = nullptr;
std::shared_ptr<Kasumi::Shader> Kasumi::Shader::DefaultInstancePointShader = nullptr;
std::shared_ptr<Kasumi::Shader> Kasumi::Shader::DefaultFrameShader = nullptr;
std::shared_ptr<Kasumi::Shader> Kasumi::Shader::Default2DShader = nullptr;
std::shared_ptr<Kasumi::Shader> Kasumi::Shader::DefaultSimpleMeshShader = nullptr;

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

Kasumi::Shader::~Shader()
{
	glUseProgram(0);
	glDeleteProgram(ID);
}
void Kasumi::Shader::use() const
{
	glUseProgram(ID);
}
void Kasumi::Shader::Init()
{
	if (DefaultMeshShader == nullptr)
	{
		std::string vertex_src = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aColor;
layout (location = 4) in vec3 aTengent;
layout (location = 5) in vec3 aBiTengent;
layout (location = 6) in uint id;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 Color;
} vs_out;

void main()
{
    vs_out.FragPos = aPos;
    vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;
    vs_out.TexCoords = aTexCoords;
    vs_out.Color = aColor;
    gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
		)";
		std::string fragment_src = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform bool is_colored;
uniform bool is_textured;
uniform bool is_framebuffer;

uniform int diffuse_texture_num;
uniform int specular_texture_num;
uniform int normal_texture_num;
uniform int height_texture_num;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 Color;
} fs_in;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    vec3 out_color = vec3(0.0f, 0.0f, 0.0f);
    if (is_colored)
    {
        out_color += fs_in.Color;
    }

    if (is_textured)
    {
        out_color += texture(texture_diffuse1, fs_in.TexCoords).rgb;// diffuse map
    }

    // blinn-phong lighting
    vec3 ambient = 0.1 * out_color;// ambient

    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    vec3 norm = normalize(fs_in.Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * out_color;// diffuse

    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = spec * vec3(0.5, 0.5, 0.5);// specular

    out_color = ambient + diffuse + specular;

    float alpha = 1.0f;
    if (is_framebuffer) alpha = 0.5;

    FragColor = vec4(out_color, alpha);
}
		)";
		DefaultMeshShader = std::make_shared<Shader>(vertex_src.c_str(), fragment_src.c_str());
	}
	if (DefaultInstanceShader == nullptr)
	{
		std::string vertex_src = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aColor; // deprecated
layout (location = 4) in vec3 aTengent;
layout (location = 5) in vec3 aBiTengent;
layout (location = 6) in uint id;
layout (location = 7) in mat4 aInstanceMatrix;
layout (location = 11) in vec4 aInstColor;

uniform mat4 projection;
uniform mat4 view;
//uniform mat4 model; we don't need this

uniform int inst_id;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 Color;
} vs_out;

flat out int instanceID;

void main()
{
    vs_out.FragPos = aPos;
    vs_out.Normal = mat3(transpose(inverse(aInstanceMatrix))) * aNormal;
    vs_out.TexCoords = aTexCoords;
    vs_out.Color = aInstColor.xyz;
    gl_Position = projection * view * aInstanceMatrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);

    instanceID = gl_InstanceID;

    if (inst_id == gl_InstanceID)
    {
        vs_out.Color = vec3(1.0, 0.3, 0.3);
    }
}
		)";
		std::string fragment_src = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 Color;

uniform bool is_colored;
uniform bool is_textured;
uniform bool is_framebuffer;
uniform bool is_random_color;

uniform int diffuse_texture_num;
uniform int specular_texture_num;
uniform int normal_texture_num;
uniform int height_texture_num;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 Color;
} fs_in;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform bool highlight_mode;
flat in int instanceID;

void main()
{
    vec3 out_color = vec3(0.0f, 0.0f, 0.0f);
    if (is_colored)
    {
        out_color += fs_in.Color;
    }

    if (is_textured)
    {
        out_color += texture(texture_diffuse1, fs_in.TexCoords).rgb;// diffuse map
    }

    // blinn-phong lighting
    vec3 ambient = 0.1 * out_color;// ambient

    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    vec3 norm = normalize(fs_in.Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * out_color;// diffuse

    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = spec * vec3(0.5, 0.5, 0.5);// specular

    out_color = ambient + diffuse + specular;

    float alpha = 1.0f;
    if (is_framebuffer) alpha = 0.5;

    if (highlight_mode) out_color *= 2;

    if (is_random_color) out_color = vec3((instanceID * 3 + 1) % 256 / 256.f, (instanceID * 5 + 1) % 256 / 256.f, (instanceID * 7 + 1) % 256 / 256.f);

    FragColor = vec4(out_color, alpha);
}
		)";
		DefaultInstanceShader = std::make_shared<Shader>(vertex_src.c_str(), fragment_src.c_str());
	}
	if (DefaultLineShader == nullptr)
	{
		std::string vertex_src = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 Color;

void main()
{
    Color = aColor;
    gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
		)";
		std::string fragment_src = R"(
#version 330 core
out vec4 FragColor;
in vec3 Color;

uniform float opacity;

void main()
{
    FragColor = vec4(Color, opacity);
}
		)";
		DefaultLineShader = std::make_shared<Shader>(vertex_src.c_str(), fragment_src.c_str());
	}
	if (DefaultInstanceLineShader == nullptr)
	{
		std::string vertex_src = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in mat4 aInstanceMatrix;

uniform mat4 projection;
uniform mat4 view;

out vec3 Color;

void main()
{
    Color = aColor;
    gl_Position = projection * view * aInstanceMatrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
		)";
		std::string fragment_src = R"(
#version 330 core
out vec4 FragColor;
in vec3 Color;

uniform float opacity;

void main()
{
    FragColor = vec4(Color, opacity);
}
		)";
		DefaultInstanceLineShader = std::make_shared<Shader>(vertex_src.c_str(), fragment_src.c_str());
	}
	if (DefaultPointShader == nullptr) // we can use the same shader for point and line
	{
		std::string vertex_src = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 Color;

void main()
{
    Color = aColor;
    gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
		)";
		std::string fragment_src = R"(
#version 330 core
out vec4 FragColor;
in vec3 Color;

uniform float opacity;

void main()
{
    FragColor = vec4(Color, opacity);
}
		)";
		DefaultPointShader = std::make_shared<Shader>(vertex_src.c_str(), fragment_src.c_str());
	}
	if (DefaultInstancePointShader == nullptr)
	{
		std::string vertex_src = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in mat4 aInstanceMatrix;

uniform mat4 projection;
uniform mat4 view;

out vec3 Color;

void main()
{
    Color = aColor;
    gl_Position = projection * view * aInstanceMatrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
		)";
		std::string fragment_src = R"(
#version 330 core
out vec4 FragColor;
in vec3 Color;

uniform float opacity;

void main()
{
    FragColor = vec4(Color, opacity);
}
		)";
		DefaultInstancePointShader = std::make_shared<Shader>(vertex_src.c_str(), fragment_src.c_str());
	}
	if (DefaultFrameShader == nullptr)
	{
		std::string vertex_src = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    TexCoords = aTexCoords;
}
		)";
		std::string fragment_src = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
    FragColor = texture(screenTexture, TexCoords);
}
		)";
		DefaultFrameShader = std::make_shared<Shader>(vertex_src.c_str(), fragment_src.c_str());
	}
	if (Default2DShader == nullptr)
	{
		std::string vertex_src = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

out vec3 Color;

uniform vec2 pos;
uniform float rot;
uniform vec2 scl;

uniform vec2 iResolution;

void main()
{
    vec2 pos_scaled = aPos * scl;
    vec2 pos_rotated = vec2(pos_scaled.x * cos(rot) - pos_scaled.y * sin(rot), pos_scaled.x * sin(rot) + pos_scaled.y * cos(rot));
    vec2 pos_world = pos_rotated + pos;
    gl_Position = vec4(pos_world.x / iResolution.x * 2, pos_world.y / iResolution.y * 2, 0.0, 1.0);
    Color = aColor;
}
		)";
		std::string fragment_src = R"(
#version 330 core
out vec4 FragColor;

in vec3 Color;

void main()
{
    FragColor = vec4(Color, 1.0f);
}
		)";
		Default2DShader = std::make_shared<Shader>(vertex_src.c_str(), fragment_src.c_str());
	}
	if (DefaultSimpleMeshShader == nullptr)
	{
		std::string vertex_src = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aColor;
layout (location = 4) in vec3 aTengent;
layout (location = 5) in vec3 aBiTengent;
layout (location = 6) in uint id;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 Color;
} vs_out;

void main()
{
    vs_out.FragPos = aPos;
    vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;
    vs_out.TexCoords = aTexCoords;
    vs_out.Color = aColor;
    gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
		)";
		std::string fragment_src = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform bool is_colored;
uniform bool is_textured;
uniform bool is_framebuffer;

uniform int diffuse_texture_num;
uniform int specular_texture_num;
uniform int normal_texture_num;
uniform int height_texture_num;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 Color;
} fs_in;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    vec3 out_color = vec3(0.0f, 0.0f, 0.0f);
    if (is_colored)
    {
        out_color += fs_in.Color;
    }

    if (is_textured)
    {
        out_color += texture(texture_diffuse1, fs_in.TexCoords).rgb;// diffuse map
    }

    float alpha = 1.0f;
    if (is_framebuffer) alpha = 0.5;

    FragColor = vec4(out_color, alpha);
}
		)";
		DefaultSimpleMeshShader = std::make_shared<Shader>(vertex_src.c_str(), fragment_src.c_str());
	}
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

void Kasumi::Shader::uniform(const std::string &name, bool value) const
{
	use();
	glUniform1i(glGetUniformLocation(ID, name.c_str()), static_cast<int>(value));
}
void Kasumi::Shader::uniform(const std::string &name, int value) const
{
	use();
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void Kasumi::Shader::uniform(const std::string &name, unsigned int value) const
{
	use();
	glUniform1ui(glGetUniformLocation(ID, name.c_str()), value);
}
void Kasumi::Shader::uniform(const std::string &name, float value) const
{
	use();
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
void Kasumi::Shader::uniform(const std::string &name, const mVector2 &value) const
{
	use();
	glUniform2f(glGetUniformLocation(ID, name.c_str()), static_cast<float>(value.x()), static_cast<float>(value.y()));
}
void Kasumi::Shader::uniform(const std::string &name, const mVector3 &value) const
{
	use();
	glUniform3f(glGetUniformLocation(ID, name.c_str()), static_cast<float>(value.x()), static_cast<float>(value.y()), static_cast<float>(value.z()));
}
void Kasumi::Shader::uniform(const std::string &name, const mVector4 &value) const
{
	use();
	glUniform4f(glGetUniformLocation(ID, name.c_str()), static_cast<float>(value.x()), static_cast<float>(value.y()), static_cast<float>(value.z()), static_cast<float>(value.w()));
}
void Kasumi::Shader::uniform(const std::string &name, const mMatrix3x3 &value) const
{
	use();
	glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, value.as_float().data());
}
void Kasumi::Shader::uniform(const std::string &name, const mMatrix4x4 &value) const
{
	use();
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, value.as_float().data());
}
void Kasumi::Shader::uniform(const std::string &name, const std::vector<unsigned int> &value, size_t size) const
{
	std::vector<GLfloat> v;
	v.resize(size);
	std::fill(v.begin(), v.end(), 0);
	for (int i = 0; i < value.size(); ++i)
		v[i] = static_cast<GLfloat>(value[i]);

	use();
	glUniform1fv(glGetUniformLocation(ID, name.c_str()), static_cast<GLsizei>(size), v.data());
}
