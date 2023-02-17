#pragma once
#include <antartar/log.hpp>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <tl/expected.hpp>
#include <utility>
#include <vector>

namespace antartar::file {
enum [[nodiscard]] status{ok, failed_to_open};

inline auto read(const std::filesystem::path& path)
    -> tl::expected<std::pmr::vector<std::byte>, status>
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        return tl::make_unexpected(status::failed_to_open);
    }
    const auto file_size = static_cast<size_t>(file.tellg());
    std::pmr::vector<std::byte> buffer(file_size);
    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
    return buffer;
}

namespace path {
constexpr inline auto join(auto... args)
{
    return (std::filesystem::path{args} / ...);
}
} // namespace path

} // namespace antartar::file
