#pragma once
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <utility>
#include <vector>
#include <antartar/log.hpp>

namespace antartar::file {
inline std::pmr::vector<std::byte> read(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error(
            log_message(fmt::format("failed to open file: {}!", path.string())));
    }
    const auto file_size = static_cast<size_t>(file.tellg());
    std::pmr::vector<std::byte> buffer(file_size);
    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
    return buffer; 
}
} // namespace antartar::file