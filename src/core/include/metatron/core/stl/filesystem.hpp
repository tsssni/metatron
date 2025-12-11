#pragma once
#include <metatron/core/stl/singleton.hpp>
#include <metatron/core/stl/optional.hpp>
#include <metatron/core/stl/print.hpp>
#include <filesystem>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <ranges>

namespace mtt::stl {
    #ifndef MTT_PREFIX
        #define MTT_PREFIX ""
    #endif

    template<typename T>
    concept buffer = requires {false
    || std::is_convertible_v<T, std::span<byte const>>
    || std::is_convertible_v<T, std::string_view>;
    };

    using path = std::filesystem::path;

    struct filesystem final: stl::singleton<filesystem> {
        path home = "";
        path cache = "";
        path prefix = "";
        path share = "";

        filesystem() noexcept {
            home = std::getenv("HOME");
            cache = home / ".cache/metatron";
            std::filesystem::create_directories(cache);
            prefix = MTT_PREFIX;
            share = prefix / "share/metatron";
            data.push_back(share);
        }

        auto static push(cref<path> path) noexcept -> void {
            instance().data.push_back(path);
        }

        auto static find(cref<path> path) noexcept -> stl::path {
            for (auto const& base : instance().data | std::views::reverse) {
                auto full = base / path;
                if (std::filesystem::exists(full)) return full;
            }
            stl::abort("{} not found", path.c_str());
            return {};
        }

        auto static hit(cref<path> path) noexcept -> opt<stl::path> {
            auto cache = instance().cache / path;
            if (std::filesystem::exists(cache)) return cache;
            return {};
        }

        auto static load(
            cref<path> path, std::ios::openmode mode = {}
        ) noexcept -> std::vector<byte> {
            auto size = std::filesystem::file_size(path);
            auto stream = std::ifstream{path, mode};
            auto buffer = std::vector<byte>(size);
            stream.read(mut<char>(buffer.data()), buffer.size());
            return buffer;
        }

        template<buffer T>
        auto static store(
            cref<path> path, T buffer, std::ios::openmode mode = {}
        ) noexcept -> void {
            std::filesystem::create_directory(path.parent_path());
            auto stream = std::ofstream{path, mode};
            stream.write(view<char>(buffer.data()), buffer.size());
        }

    private:
        std::vector<std::filesystem::path> data;
    };
}
