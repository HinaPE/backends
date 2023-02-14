#include "GLFW/glfw3.h"

#include "platform.h"

class TutorialApp : public Kasumi::App
{
public:
	TutorialApp(int width, int height, const std::string &title) : App(width, height, title) {}
};

auto main() -> int
{
	std::make_shared<TutorialApp>(1024, 768, "00Platform")->launch();
	return 0;
}