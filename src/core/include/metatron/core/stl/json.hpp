#pragma once
#include <metatron/core/stl/filesystem.hpp>
#include <glaze/glaze.hpp>

namespace mtt::stl {
    struct json final {
        template<typename T, buffer U>
        auto static load(U json, T&& buffer) noexcept -> void {
            if (auto e = glz::read_json(buffer, json)) {
                std::println("glaze error: {}", glz::format_error(e));
                std::abort();
            }
        }

        template<typename T>
        auto static load(path path, T&& buffer) noexcept -> void {
            auto json = stl::filesystem::instance().load(path);
            if (auto e = glz::read_json(buffer, json)) {
                std::println(
                    "load {} with glaze error: {}",
                    path.c_str(), glz::format_error(e)
                );
                std::abort();
            }
        }

        template<typename T>
        auto static store(T&& buffer) noexcept -> std::vector<byte> {
            auto json = std::vector<byte>{};
            if (auto e = glz::write_json(buffer, json)) {
                std::println("glaze error: {}", glz::format_error(e));
                std::abort();
            }
            return json;
        }

        template<typename T>
        auto static store(path path, T&& buffer) noexcept -> void {
            auto json = std::vector<byte>{};
            if (auto e = glz::write_json(buffer, json)) {
                std::println(
                    "store {} with glaze error: {}",
                    path.c_str(), glz::format_error(e)
                );
                std::abort();
            }
            stl::filesystem::instance().store(path, json);
        }
    };
}
