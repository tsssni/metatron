#pragma once
#include <metatron/core/stl/filesystem.hpp>
#include <glaze/glaze.hpp>

namespace mtt::core {

struct json final {
    template<typename T, typename U>
    auto static load(U json, T&& buffer) noexcept -> void requires(std::convertible_to<U, std::span<byte const>> || std::convertible_to<U, std::string_view>);
    template<typename T>
    auto static load(path path, T&& buffer) noexcept -> void;
    template<typename T>
    auto static store(T&& buffer) noexcept -> std::vector<byte>;
    template<typename T>
    auto static store(path path, T&& buffer) noexcept -> void;
};

template<typename T, typename U>
auto json::load(U json, T&& buffer) noexcept -> void requires(std::convertible_to<U, std::span<byte const>> || std::convertible_to<U, std::string_view>) {
    if (auto e = glz::read_json(std::forward<T>(buffer), json))
        abort("glaze error: {}", glz::format_error(e));
}

template<typename T>
auto json::load(path path, T&& buffer) noexcept -> void {
    auto json = filesystem::load(path);
    if (auto e = glz::read_json(std::forward<T>(buffer), json))
        abort(
            "load {} with glaze error: {}",
            path.c_str(), glz::format_error(e)
        );
}

template<typename T>
auto json::store(T&& buffer) noexcept -> std::vector<byte> {
    auto json = std::vector<byte>{};
    if (auto e = glz::write_json(buffer, json))
        abort("glaze error: {}", glz::format_error(e));
    return json;
}

template<typename T>
auto json::store(path path, T&& buffer) noexcept -> void {
    auto json = std::vector<byte>{};
    if (auto e = glz::write_json(buffer, json))
        abort(
            "store {} with glaze error: {}",
            path.c_str(), glz::format_error(e)
        );
    filesystem::store(path, json);
}

}
