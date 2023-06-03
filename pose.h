#ifndef BACKENDS_POSE_H
#define BACKENDS_POSE_H

// Copyright (c) 2023 Xayah Hina
// MPL-2.0 license

#include "common.h"

namespace Kasumi
{
struct Pose
{
	mVector3 position;
	mVector3 euler;
	mVector3 scale;

	explicit Pose(mVector3 position = {0, 0, 0}, mVector3 euler = {0, 0, 0}, mVector3 scale = {1, 1, 1}) : position(std::move(position)), euler(std::move(euler)), scale(std::move(scale)) {}
	inline auto get_model_matrix() const -> mMatrix4x4 { return mMatrix4x4::make_translation_matrix(position) * mQuaternion(euler.x(), euler.y(), euler.z()).matrix4x4() * mMatrix4x4::make_scale_matrix(scale); }
    inline auto get_rotation_matrix() const -> mMatrix4x4 { return mQuaternion(euler.x(), euler.y(), euler.z()).matrix4x4(); }
    inline auto get_rotation_matrix_3() const -> mMatrix3x3 { return mQuaternion(euler.x(), euler.y(), euler.z()).matrix3x3(); }
};

struct Pose2D
{
	mVector2 position;
	float rotation;
	mVector2 scale;

	explicit Pose2D(mVector2 position = {0, 0}, float rotation = 0, mVector2 scale = {1, 1}) : position(std::move(position)), rotation(rotation), scale(std::move(scale)) {}
};
} // namespace Kasumi

#endif //BACKENDS_POSE_H
