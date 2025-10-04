#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <metatron/scene/ecs/stage.hpp>
#include <metatron/scene/serde/serde.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::ecs {
    template<typename T>
    struct Dirty_Mark final {};

    struct Hierarchy final: stl::capsule<Hierarchy> {
        Registry registry;
        std::vector<mut<Stage>> stages;

        struct Impl;
        Hierarchy() noexcept;

        mut<Hierarchy> static instance;
        auto activate() noexcept -> void;

        auto create(std::string const& path) noexcept -> Entity;
        auto entity(std::string const& path) const noexcept -> Entity;
        auto path(Entity entity) const noexcept -> std::string const&;

        auto root() const noexcept -> Entity;
        auto parent(Entity entity) const noexcept -> Entity;
        auto children(Entity entity) const noexcept -> std::vector<Entity> const&;

        auto init() noexcept -> void;
        auto update() noexcept -> void;

        template<typename T>
        auto static serde(std::string const& type) noexcept -> void {
            auto sanitized_type = type;
            std::ranges::transform(
                sanitized_type,
                sanitized_type.begin(),
                ::tolower
            );

            auto fr = [](ecs::Entity e, glz::raw_json const& s) -> void {
                auto d = T{};
                if (auto er = glz::read_json<T>(d, s.str); er) {
                    std::println("desrialize {} with glaze error: {}", s.str, glz::format_error(er));
                    std::abort();
                } else {
                    ecs::Hierarchy::instance->attach(e, std::move(d));
                }
            };
            auto fw = [sanitized_type]() -> std::vector<serde::json> {
                auto v = std::vector<serde::json>{};
                auto& r = ecs::Hierarchy::instance->registry;
                for (auto e: r.view<T>()) {
                    auto s = glz::write_json(r.get<T>(e));
                    if (!s) {
                        std::println(
                            "failed to serialize component {} on {}",
                            sanitized_type, ecs::Hierarchy::instance->path(e)
                        );
                        std::abort();
                    }
                    v.emplace_back(e, sanitized_type, s.value());
                }
                return v;
            };
            ecs::Hierarchy::instance->enable(sanitized_type, fr, fw);
        }
        auto enable(
            std::string const& type,
            std::function<void(ecs::Entity e, glz::raw_json const& s)> fr,
            std::function<std::vector<serde::json>()> fw
        ) noexcept -> void;
        auto read(std::string path) noexcept -> void;
        auto write(std::string path) noexcept -> void;

        template<typename T>
        auto attach(Entity entity, T&& component = {}) noexcept -> void {
            registry.emplace<Dirty_Mark<T>>(entity);
            registry.emplace<T>(entity, std::forward<T>(component));
        }

        template<typename T>
        auto fetch(Entity entity) noexcept -> T& {
            return registry.get<T>(entity);
        }

        template<typename T>
        auto detach(Entity entity) noexcept -> void {
            registry.emplace<Dirty_Mark<T>>(entity);
            registry.erase<T>(entity);
        }
    };

    #define MTT_SERDE(T) ecs::Hierarchy::serde<compo::T>(#T)
}
