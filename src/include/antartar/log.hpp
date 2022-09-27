#pragma once

#include <fmt/format.h>
#include <source_location>
#include <string_view>

using namespace std::string_view_literals;

namespace antartar {
inline auto
log_message(std::convertible_to<std::string_view> auto message,
            std::source_location current_loc = std::source_location::current())
{
    return fmt::format("[file: {}][line: {}] {}",
                       current_loc.file_name(),
                       current_loc.line(),
                       message);
}

inline auto
log(std::convertible_to<std::string_view> auto message,
    std::source_location current_loc = std::source_location::current())
{
    auto output = log_message(message, current_loc);
    fmt::print("{}\n", output);
}
} // namespace antartar