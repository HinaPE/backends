#ifndef BACKENDS_SHADER_H
#define BACKENDS_SHADER_H

// Copyright (c) 2023 Xayah Hina
// MPL-2.0 license

#include "common.h"

namespace Kasumi
{
class Shader final : public HinaPE::CopyDisable
{
public:
	unsigned int ID;

	static void Init();
	static std::shared_ptr<Shader> DefaultMeshShader;
	static std::shared_ptr<Shader> DefaultInstanceShader;
	static std::shared_ptr<Shader> DefaultLineShader;
	static std::shared_ptr<Shader> DefaultInstanceLineShader;
	static std::shared_ptr<Shader> DefaultPointShader;
	static std::shared_ptr<Shader> DefaultFrameShader;
	static std::shared_ptr<Shader> Default2DShader;
	static std::shared_ptr<Shader> DefaultSimpleMeshShader;

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
} // namespace Kasumi

#endif //BACKENDS_SHADER_H
