#include <antartar/app.hpp>
#include <fmt/format.h>

int main()
{
    fmt::print("hello ocean of antartar\n");

    try {
        antartar::app app;
        app.run();
    }
    catch (const std::exception& e) {
        fmt::print("{}\n", e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}