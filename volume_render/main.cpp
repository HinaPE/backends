#include "volume_render.h"
#include "../api.h"

#include "../objects/sphere.h"

class TestApp : public Kasumi::App
{
protected:
	void update(double dt) final {  _sphere.render(); }

private:
	Kasumi::SphereObject _sphere;
};

auto main() -> int
{
	TestApp app;
	app.clean_mode();
	app.launch();
	return 0;
}