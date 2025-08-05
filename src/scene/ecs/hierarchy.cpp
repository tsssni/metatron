#include <metatron/scene/ecs/hierarchy.hpp>
#include <metatron/scene/ecs/stage.hpp>
#include <metatron/scene/serde/serde.hpp>
#include <unordered_map>
#include <print>

namespace mtt::ecs {
	mut<Hierarchy> Hierarchy::instance{nullptr};

	struct Hierarchy::Impl final {
		mut<Hierarchy> hierarchy;

		std::unordered_map<std::string, Entity> entities;
		std::unordered_map<Entity, std::string> pathes;
		std::unordered_map<Entity, Entity> fathers;
		std::unordered_map<Entity, std::vector<Entity>> sons;

		std::unordered_map<std::string, std::function<void(Entity e, std::string const& s)>> frs;
		std::unordered_map<std::string, std::function<std::vector<serde::json>()>> fws;

		auto plant() noexcept -> void {
			entities["/"] = hierarchy->registry.create();
			fathers[entities["/"]] = ecs::null;
		}

		auto create(std::string const& path) noexcept -> Entity {
			if (entities.find(path) != entities.end()) {
				return entities[path];
			}

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
		auto fetch(
			std::unordered_map<T, U> const& map, 
		    std::conditional_t<sizeof(T) <= sizeof(void*) && std::is_trivially_copyable_v<T>, T, T const&> key
		) const -> std::conditional_t<sizeof(U) <= sizeof(void*) && std::is_trivially_copyable_v<U>, U, U const&> {
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

	auto Hierarchy::activate() noexcept -> void {
		Hierarchy::instance = this;
	}

	auto Hierarchy::create(std::string const& path) noexcept -> Entity {
		return impl->create(path);
	}

	auto Hierarchy::entity(std::string const& path) const noexcept -> Entity {
		return impl->fetch(impl->entities, path);
	}

	auto Hierarchy::path(Entity entity) const noexcept -> std::string const& {
		return impl->fetch(impl->pathes, entity);
	}

	auto Hierarchy::root() const noexcept -> Entity {
		return impl->entities.at("/");
	}

	auto Hierarchy::parent(Entity entity) const noexcept -> Entity {
		return impl->fetch(impl->fathers, entity);
	}

	auto Hierarchy::children(Entity entity) const noexcept -> std::vector<Entity> const& {
		return impl->fetch(impl->sons, entity);
	}

	auto Hierarchy::enable(
		std::string const& type,
		std::function<void(Entity e, std::string const& s)> fr,
		std::function<std::vector<serde::json>()> fw
	) noexcept -> void {
		impl->frs.emplace(type, std::move(fr));
		impl->fws.emplace(type, std::move(fw));
	}

	auto Hierarchy::read(std::string path) noexcept -> void {
	}

	auto Hierarchy::write(std::string path) noexcept -> void {
		auto jsons = std::vector<serde::json>{};
		for (auto [type, fw]: impl->fws) {
			auto js = fw();
			std::ranges::move(js, std::back_inserter(jsons));
		}

		auto serialized = glz::write_json(jsons);
		if (!serialized) {
			std::println("failed to serialize hierarchy");
			std::abort();
		}

		auto f = std::ofstream{path};
		if (!f.is_open()) {
			std::println("failed to write serialized hierarchy to {}", path);
			std::abort();
		}
		f << glz::prettify_json(serialized.value());
	}

	auto Hierarchy::init() noexcept -> void {
		for (auto& stage: stages) {
			stage->init();
		}
	}

	auto Hierarchy::update() noexcept -> void {
		for (auto& stage: stages) {
			stage->update();
		}
	}
}
