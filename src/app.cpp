#include <antartar/app.hpp>
#include <fmt/format.h>

namespace antartar {
void app::run()
{
    fmt::print("running antartar\n");

    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr,
                                           std::addressof(extension_count),
                                           nullptr);
    log(fmt::format("vulkan supports {} extensions", extension_count));

    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        draw_frame_();
    }
    window_.wait_idle();
}

void app::draw_frame_() { window_.draw_frame(); }
} // namespace antartar