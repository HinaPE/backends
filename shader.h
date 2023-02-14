#ifndef KASUMI_SHADER_H
#define KASUMI_SHADER_H

// Copyright (c) 2023 Xayah Hina
// MPL-2.0 license

// Dependency:
// - Math Backend

#include "common.h"

#include <vector>
#include <string>
#include <memory>

namespace Kasumi
{
class Shader final : public HinaPE::CopyDisable
{
public:
	unsigned int ID;

	static void Init();
	static std::unique_ptr<Shader> DefaultMeshShader;
	static std::unique_ptr<Shader> DefaultInstanceShader;
	static std::unique_ptr<Shader> DefaultLineShader;
	static std::unique_ptr<Shader> DefaultPointShader;
	static std::unique_ptr<Shader> DefaultFrameShader;

public:
	Shader(const std::string &vertex_path, const std::string &fragment_path);
	Shader(const std::string &vertex_path, const std::string &fragment_path, const std::string &geometry_path);
	Shader(const char *vertex_src, const char *fragment_src);
	Shader(const char *vertex_src, const char *fragment_src, const char *geometry_src);
	~Shader();

	void uniform(const std::string &name, bool value) const;
	void uniform(const std::string &name, int value) const;
	void uniform(const std::string &name, unsigned int value) const;
	void uniform(const std::string &name, float value) const;
	void uniform(const std::string &name, const mVector2 &value) const;
	void uniform(const std::string &name, const mVector3 &value) const;
	void uniform(const std::string &name, const mVector4 &value) const;
	void uniform(const std::string &name, const mMatrix3x3 &value) const;
	void uniform(const std::string &name, const mMatrix4x4 &value) const;

	void use() const;

private:
	static void _validate(unsigned int shader, const std::string &type);
};
using ShaderPtr = std::shared_ptr<Shader>;
}

#endif //KASUMI_SHADER_H
