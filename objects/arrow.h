#ifndef HINAPE_ARROW_H
#define HINAPE_ARROW_H

// Copyright (c) 2023 Xayah Hina
// MPL-2.0 license

#include "object3D.h"

namespace Kasumi
{
class ArrowObject : public ObjectMesh3D
{
public:
	ArrowObject();

protected:
	void _update_surface() final;

	mVector3 _origin = mVector3::Zero();
	mVector3 _direction = mVector3::UnitZ();
};
using ArrowObjectPtr = std::shared_ptr<ArrowObject>;
} // namespace Kasumi

#endif //HINAPE_ARROW_H
