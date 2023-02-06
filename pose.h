#ifndef KASUMI_POSE_H
#define KASUMI_POSE_H

#include "math_api.h"

namespace Kasumi
{
struct Pose
{
	mVector3 position = {0, 0, 0};
	mVector3 euler = {0, 0, 0};
	mVector3 scale = {1, 1, 1};

	inline auto get_model_matrix() const -> mMatrix4x4 { return mMatrix4x4::make_translation_matrix(position) * mQuaternion(euler.x(), euler.y(), euler.z()).matrix4x4() * mMatrix4x4::make_scale_matrix(scale); }
};
}
#endif //KASUMI_POSE_H
