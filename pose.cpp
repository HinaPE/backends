#include "pose.h"

auto Kasumi::Pose::get_model_matrix() const -> mMatrix4x4 { return mMatrix4x4::makeTranslationMatrix(position) * mQuaternion(euler.x, euler.y, euler.z).matrix4() * mMatrix4x4::makeScaleMatrix(scale); }
