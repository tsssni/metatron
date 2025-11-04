#include <metatron/render/scene/hierarchy.hpp>
#include <metatron/render/scene/serde.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/print.hpp>
#include <unordered_map>

namespace mtt::scene {
    struct Hierarchy::Impl final {
        mut<Hierarchy> hierarchy;
        std::vector<Hierarchy::filter_function> filters;

        std::unordered_map<std::string, Entity> entities;
        std::unordered_map<Entity, std::string> pathes;
        std::unordered_map<Entity, Entity> fathers;
        std::unordered_map<Entity, std::vector<Entity>> sons;

        auto plant() noexcept -> void {
            entities["/"] = hierarchy->registry.create();
            fathers[entities["/"]] = entt::null;
        }

        auto create(std::string_view pv) noexcept -> Entity {
            auto path = std::string{pv};
            if (entities.find(path) != entities.end()) return entities[path];

            auto slash = path.find_last_of('/');
            if (slash == std::string::npos) {
                std::println("ecs: invalid path {}", path);
                std::abort();
            }
            
            auto parent_path = slash == 0 ? "/" : path.substr(0, slash);
            auto parent = create(parent_path);

            auto entity = hierarchy->registry.create();
            fathers[entity] = entities[parent_path];
            sons[parent].push_back(entity);
            sons[entity] = {};

            pathes[entity] = path;
            entities[path] = entity;
            return entity;
        }

        template<typename T, typename U>
        auto fetch(std::unordered_map<T, U> const& map, T const& key) const -> U const& {
            auto it = map.find(key);
            if (it != map.end()) {
                return it->second;
            } else {
                std::println("ecs: invalid key");
                std::abort();
            }
        }
    };

    Hierarchy::Hierarchy() noexcept {
        impl->hierarchy = this;
        impl->plant();
    }

    auto Hierarchy::entity(std::string_view path) noexcept -> Entity {
        return impl->create(path);
    }

    auto Hierarchy::path(Entity entity) const noexcept -> std::string_view {
        return impl->fetch(impl->pathes, entity);
    }

    auto Hierarchy::root() const noexcept -> Entity {
        return impl->entities.at("/");
    }

    auto Hierarchy::parent(Entity entity) const noexcept -> Entity {
        return impl->fetch(impl->fathers, entity);
    }

    auto Hierarchy::children(Entity entity) const noexcept -> std::span<Entity const> {
        return impl->fetch(impl->sons, entity);
    }

    auto Hierarchy::filter(filter_function f) noexcept -> void {
        impl->filters.push_back(f);
    }

    auto Hierarchy::populate(std::string_view path) noexcept -> void {
        stl::filesystem::instance().push(path);

        auto jsons = std::vector<scene::json>{};
        if (auto e = glz::read_file_json(jsons, std::string{path} + "scene.json", std::string{}); e)
            std::println("read scene {} with glaze error: {}", path, glz::format_error(e));

        auto bins = std::unordered_map<std::string, std::vector<json>>{};
        for (auto& j: jsons) bins[j.type].push_back(std::move(j));
        for (auto& f: impl->filters) f(bins);
    }
}
