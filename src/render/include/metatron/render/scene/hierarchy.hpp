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

        using filter_function = std::function<auto (json const&, std::mutex&) -> void>;
        auto bundle(std::span<std::string_view> types) noexcept -> void;
        auto filter(std::string_view type, filter_function f) noexcept -> void;
        auto populate(std::string_view path) noexcept -> void;

        template<typename F, typename T>
        auto attach(Entity entity, T&& component = {}) noexcept -> void {
            auto idx = stl::poly_vector<F>::instance().push_back(std::forward<T>(component));
            registry.emplace(entity, stl::proxy<F>{idx});
        }

        template<typename F>
        auto fetch(Entity entity) noexcept -> stl::proxy<F> {
            return registry.get<stl::proxy<F>>(entity);
        }

        template<typename F>
        auto detach(Entity entity) noexcept -> void {
            registry.erase<stl::proxy<F>>(entity);
        }
    };
}
