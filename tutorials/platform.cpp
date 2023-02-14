#include "GLFW/glfw3.h"

#include "platform.h"

auto main() -> int
{
	std::make_shared<Kasumi::App>(Kasumi::App::Opt())->launch();
	return 0;
}
