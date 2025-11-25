#pragma once
#include <metatron/core/stl/singleton.hpp>
#include <metatron/core/stl/optional.hpp>
#include <vector>
#include <cstdlib>
#include <ranges>
#include <filesystem>

namespace mtt::stl {
    #ifndef MTT_PREFIX
        #define MTT_PREFIX ""
    #endif

    struct filesystem final: stl::singleton<filesystem> {
        std::filesystem::path home = "";
        std::filesystem::path cache = "";
        std::filesystem::path prefix = "";
        std::filesystem::path share = "";

        filesystem() noexcept {
            home = std::getenv("HOME");
            cache = home / ".cache/metatron";
            std::filesystem::create_directories(cache);

            prefix = MTT_PREFIX;
            share = prefix / "share/metatron";
            push(share.c_str());
        }

        auto push(std::string_view path) noexcept -> void { data.push_back(path); }

        auto find(std::string_view path) noexcept -> opt<std::string> {
            for (auto const& base : data | std::views::reverse) {
                auto full = base / path;
                if (std::filesystem::exists(full)) return full.string();
            }
            return {};
        }

    private:
        std::vector<std::filesystem::path> data;
    };
}
