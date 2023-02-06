#include "platform.h"

class TestApp : public Kasumi::App
{
public:
	TestApp(int width, int height, const std::string &title) : App(width, height, title) {}
};

auto main() -> int
{
	auto test = std::make_shared<TestApp>(800, 600, "Test App");
	test->launch();
	return 0;
}
