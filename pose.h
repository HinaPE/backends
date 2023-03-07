#ifndef BACKENDS_POSE_H
#define BACKENDS_POSE_H

// Copyright (c) 2023 Xayah Hina
// MPL-2.0 license

#include "common.h"

namespace Kasumi
{
struct Pose
{
	mVector3 position = {0, 0, 0};
	mVector3 euler = {0, 0, 0};
	mVector3 scale = {1, 1, 1};

	inline auto get_model_matrix() const -> mMatrix4x4 { return mMatrix4x4::make_translation_matrix(position) * mQuaternion(euler.x(), euler.y(), euler.z()).matrix4x4() * mMatrix4x4::make_scale_matrix(scale); }
};

// TODO: support 2D Pose
struct Pose2D
{
};
} // namespace Kasumi

#endif //BACKENDS_POSE_H
