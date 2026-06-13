#pragma once
#include <metatron/resource/serde/json.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::scene {
    struct Hierarchy final: stl::singleton<Hierarchy>, stl::capsule<Hierarchy> {
        struct Impl;
        Hierarchy() noexcept;

        using binmap = std::unordered_map<std::string, std::vector<json>>;
        using filter_function = std::move_only_function<auto (cref<binmap>) -> void>;
        auto static filter(filter_function f) noexcept -> void;
        auto static populate(cref<stl::path> path) noexcept -> void;
    };
}
