#ifndef KASUMI_SPHERE_H
#define KASUMI_SPHERE_H

// Copyright (c) 2023 Xayah Hina
// MPL-2.0 license

#include "object3D.h"

namespace Kasumi
{
class SphereObject : public ObjectMesh3D, public HinaPE::Geom::Sphere3
{
public:
	SphereObject();
	auto generate_surface() const -> std::vector<mVector3> override;
	void _update_surface() final;
};
using SphereObjectPtr = std::shared_ptr<SphereObject>;
} // namespace Kasumi

#endif //KASUMI_SPHERE_H
