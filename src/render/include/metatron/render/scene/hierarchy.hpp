#pragma once
#include <metatron/render/scene/entity.hpp>
#include <metatron/render/scene/json.hpp>
#include <metatron/core/stl/capsule.hpp>
#include <metatron/core/stl/singleton.hpp>

namespace mtt::scene {
    struct Hierarchy final: stl::singleton<Hierarchy>, stl::capsule<Hierarchy> {
        Registry registry;

        struct Impl;
        Hierarchy() noexcept;

        auto entity(std::string_view path) noexcept -> Entity;
        auto path(Entity entity) const noexcept -> std::string_view;

        auto root() const noexcept -> Entity;
        auto parent(Entity entity) const noexcept -> Entity;
        auto children(Entity entity) const noexcept -> std::span<Entity const>;

        using binmap = std::unordered_map<std::string, std::vector<json>>;
        using filter_function = std::function<auto (cref<binmap>) -> void>;
        auto filter(filter_function f) noexcept -> void;
        auto populate(std::string_view path) noexcept -> void;
    };

    template<typename F, typename T = F>
    auto attach(Entity entity, T&& component = {}) noexcept -> tag<F> {
        auto idx = stl::vector<F>::instance().push_back(std::forward<T>(component));
        auto handle = tag<F>{idx};
        Hierarchy::instance().registry.emplace<tag<F>>(entity, handle);
        return handle;
    }

    template<typename F>
    auto exist(Entity entity) noexcept -> bool {
        return Hierarchy::instance().registry.any_of<tag<F>>(entity);
    }

    template<typename F>
    auto fetch(Entity entity) noexcept -> tag<F> {
        return Hierarchy::instance().registry.get<tag<F>>(entity);
    }

    template<typename F>
    auto detach(Entity entity) noexcept -> void {
        Hierarchy::instance().registry.erase<tag<F>>(entity);
    }

    template<typename F>
    auto entities() noexcept -> std::vector<Entity> {
        return Hierarchy::instance().registry.view<tag<F>>()
        | std::ranges::to<std::vector<Entity>>();
    }
}
