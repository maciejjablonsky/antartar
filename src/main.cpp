#include <fmt/format.h>
#include <antartar/app.hpp>

int main()
{
	fmt::print("hello ocean of antartar\n");

	try {
		antartar::app app;
		app.run();
	}
	catch (const std::exception&e)
	{
		fmt::print("{}\n", e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}