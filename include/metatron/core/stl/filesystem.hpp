#pragma once
#include <metatron/core/stl/concept.hpp>
#include <metatron/core/stl/optional.hpp>
#include <metatron/core/stl/print.hpp>
#include <metatron/core/stl/singleton.hpp>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <vector>

namespace mtt::core {

#ifndef MTT_PREFIX
#define MTT_PREFIX ""
#endif

using path = std::filesystem::path;

struct filesystem final : singleton<filesystem> {
    path home = "";
    path cache = "";
    path prefix = "";
    path share = "";

    filesystem() noexcept;

    auto static push(path const& path) noexcept -> void;
    auto static find(path const& path) noexcept -> core::path;
    auto static hit(path const& path) noexcept -> opt<core::path>;

    auto static load(
        path const& path, std::ios::openmode mode = {}, bool cstr = false
    ) noexcept -> std::vector<byte>;

    template<typename T>
    auto static store(
        path const& path, T buffer, std::ios::openmode mode = {}
    ) noexcept -> void requires(false || std::convertible_to<T, std::span<byte const>> || std::convertible_to<T, std::string_view>);

private:
    std::vector<std::filesystem::path> data;
};

inline filesystem::filesystem() noexcept {
    home = std::getenv("HOME");
    cache = home / ".cache/metatron";
    std::filesystem::create_directories(cache);
    prefix = MTT_PREFIX;
    share = prefix / "share/metatron";
    data.push_back(share);
}

auto inline filesystem::push(path const& path) noexcept -> void {
    instance().data.push_back(path);
}

auto inline filesystem::find(path const& path) noexcept -> core::path {
    for (auto const& base : instance().data | std::views::reverse) {
        auto full = base / path;
        if (std::filesystem::exists(full)) return full;
    }
    abort("{} not found", path.c_str());
    return {};
}

auto inline filesystem::hit(path const& path) noexcept -> opt<core::path> {
    auto cache = instance().cache / path;
    if (std::filesystem::exists(cache)) return cache;
    return {};
}

auto inline filesystem::load(
    path const& path, std::ios::openmode mode, bool cstr
) noexcept -> std::vector<byte> {
    auto size = std::filesystem::file_size(path);
    auto stream = std::ifstream{path, mode};
    auto buffer = std::vector<byte>(size + cstr);
    stream.read((char*)buffer.data(), buffer.size());
    if (cstr) buffer.back() = '\0';
    return buffer;
}

template<typename T>
auto filesystem::store(
    path const& path, T buffer, std::ios::openmode mode
) noexcept -> void requires(false || std::convertible_to<T, std::span<byte const>> || std::convertible_to<T, std::string_view>) {
    std::filesystem::create_directory(path.parent_path());
    auto stream = std::ofstream{path, mode};
    stream.write((char const*)buffer.data(), buffer.size());
}

}
