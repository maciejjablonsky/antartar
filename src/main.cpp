#include <fmt/format.h>
#include <antartar/app.h>

int main()
{
	fmt::print("hello ocean of antartar\n");

	try {
		antartar::App app;
		app.run();
	}
	catch (...)
	{
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}