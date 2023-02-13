#ifndef KASUMI_LIGHT_H
#define KASUMI_LIGHT_H

// Copyright (c) 2023 Xayah Hina
// MPL-2.0 license

// Dependency:
// - Math Backend
// - Shader
// - Camera

#include "shader.h"
#include "camera.h"

#include <memory>

namespace Kasumi
{
class Light final
{
public: //! ==================== Public Methods ====================
	void update(const CameraPtr &camera);
	void sync_shader(const Shader &shader) const;
	void render();
//	ShaderPtr _shader;

public:
	enum Type { Directional, Point, Spot, Count };
	struct Opt
	{
		Type type = Directional;

		// common
		mVector3 light_pos = mVector3(5.0f, 15.0f, 5.0f);
		mVector3 view_pos = mVector3(0.0f, 0.0f, 0.0f);

		// directional light
		mVector3 light_dir = mVector3(-1.0f, -1.0f, -1.0f);
	} _opt;

//! ==================== Constructors & Destructor ====================
//! - [DELETE] copy constructor & copy assignment operator
//! - [ENABLE] move constructor & move assignment operator
public:
	explicit Light(Opt opt);
	Light(const Light &src) = delete;
	Light(Light &&src) noexcept = default;
	~Light();
	void operator=(const Light &src) = delete;
	auto operator=(Light &&src) noexcept -> Light & = default;
};
using LightPtr = std::shared_ptr<Light>;
}

#endif //KASUMI_LIGHT_H
