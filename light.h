#ifndef KASUMI_LIGHT_H
#define KASUMI_LIGHT_H

// Copyright (c) 2023 Xayah Hina
// MPL-2.0 license

#include "common.h"

namespace Kasumi
{
class Light final : public HinaPE::CopyDisable
{
public:
	static void Init();
	static std::shared_ptr<Light> MainLight;

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
};
using LightPtr = std::shared_ptr<Light>;
}

#endif //KASUMI_LIGHT_H
