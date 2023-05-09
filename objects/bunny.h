#ifndef KASUMI_BUNNY_H
#define KASUMI_BUNNY_H

// Copyright (c) 2023 Xayah Hina
// MPL-2.0 license

#include "object3D.h"
namespace Kasumi
{
class BunnyObject : public ObjectMesh3D, public HinaPE::Geom::TriangleMeshSurface
{
public:
	BunnyObject();
	auto generate_surface() const -> std::vector<mVector3> override;
	void _update_surface() final;
};
using BunnyObjectPtr = std::shared_ptr<BunnyObject>;
} // namespace Kasumi

#endif //KASUMI_BUNNY_H
