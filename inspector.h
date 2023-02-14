#ifndef KASUMI_INSPECTOR_H
#define KASUMI_INSPECTOR_H

#include "imgui.h"
#include <memory>

namespace Kasumi
{
class Inspector
{
	friend class App;
	virtual void inspect() = 0;
};
using InspectorPtr = std::shared_ptr<Inspector>;
} // namespace Kasumi

#endif //KASUMI_INSPECTOR_H
