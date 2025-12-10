#pragma once
#include <metatron/render/scene/json.hpp>
#include <metatron/core/stl/capsule.hpp>
#include <metatron/core/stl/singleton.hpp>

namespace mtt::scene {
    struct Hierarchy final: stl::singleton<Hierarchy>, stl::capsule<Hierarchy> {
        struct Impl;
        Hierarchy() noexcept;

        using binmap = std::unordered_map<std::string, std::vector<json>>;
        using filter_function = std::function<void(cref<binmap>)>;
        auto filter(filter_function f) noexcept -> void;
        auto populate(cref<stl::path> path) noexcept -> void;
    };
}
