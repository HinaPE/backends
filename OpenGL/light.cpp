#include "../light.h"

std::shared_ptr<Kasumi::Light> Kasumi::Light::MainLight = nullptr;
void Kasumi::Light::Init()
{
	MainLight = std::make_shared<Light>();
}
