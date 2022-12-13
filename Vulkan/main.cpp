#include "platform.h"

class App : public KasumiVulkan::App
{
public:
	App(int width, int height, const std::string &title) : KasumiVulkan::App(width, height, title) {}
};

auto main() -> int
{
	try
	{
		std::make_shared<App>(800, 600, "Kasumi, but Vulkan")->launch();
	} catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return 0;
}