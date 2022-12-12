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

	inline auto get_model_matrix() const -> mMatrix4x4 { return mMatrix4x4::makeTranslationMatrix(position) * mQuaternion(euler.x, euler.y, euler.z).matrix4() * mMatrix4x4::makeScaleMatrix(scale); }
};
}
#endif //KASUMI_POSE_H
