#ifndef KASUMI_POSE_H
#define KASUMI_POSE_H

// Copyright (c) 2023 Xayah Hina
// MPL-2.0 license

#include "common.h"
#include "api.h"

namespace Kasumi
{
struct Pose : public INSPECTOR
{
	mVector3 position = {0, 0, 0};
	mVector3 euler = {0, 0, 0};
	mVector3 scale = {1, 1, 1};

	inline auto get_model_matrix() const -> mMatrix4x4 { return mMatrix4x4::make_translation_matrix(position) * mQuaternion(euler.x(), euler.y(), euler.z()).matrix4x4() * mMatrix4x4::make_scale_matrix(scale); }

public:
	void INSPECT() final
	{
		ImGui::Text("Transform");
		auto sliders = [&](const std::string& label, mVector3 &data, float sens)
		{
			auto data_float = data.as_float();
			ImGui::DragFloat3(label.c_str(), &data_float[0], sens);
			data = data_float.as_double();
		};
		sliders("Position", position, 0.1f);
		sliders("Rotation", euler, 0.1f);
		sliders("Scale", scale, 0.031f);
	}
};
}
#endif //KASUMI_POSE_H
