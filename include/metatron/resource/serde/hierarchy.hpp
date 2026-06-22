#pragma once
#include <metatron/resource/serde/json.hpp>
#include <metatron/core/stl/capsule.hpp>
#include <metatron/core/stl/function.hpp>

namespace mtt::scene {
    struct Hierarchy final: stl::singleton<Hierarchy>, stl::capsule<Hierarchy> {
        struct Impl;
        Hierarchy() noexcept;

        using binmap = std::unordered_map<std::string, std::vector<json>>;
        using filter_function = stl::function<auto (ref<binmap>) noexcept -> void>;
        auto static constexpr default_filter = +[](ref<binmap>) noexcept {};
        auto static filter(filter_function f) noexcept -> void;
        auto static populate(cref<stl::path> path) noexcept -> void;
    };
}
