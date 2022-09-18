#include <fmt/format.h>
#include <antartar/app.hpp>

int main()
{
	fmt::print("hello ocean of antartar\n");

	try {
		antartar::app app;
		app.run();
	}
	catch (...)
	{
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}