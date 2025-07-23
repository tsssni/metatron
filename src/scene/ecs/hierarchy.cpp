#include <metatron/scene/ecs/hierarchy.hpp>
#include <metatron/scene/ecs/stage.hpp>
#include <unordered_map>

namespace mtt::ecs {
	struct Hierarchy::Impl final {
		mut<Hierarchy> hierarchy;

		std::unordered_map<std::string, Entity> entities;
		std::unordered_map<Entity, Entity> fathers;
		std::unordered_map<Entity, std::vector<Entity>> sons;

		auto plant() noexcept -> void {
			entities["/"] = hierarchy->registry.create();
			fathers[entities["/"]] = ecs::null;
		}

		auto create(std::string const& name) noexcept -> Entity {
			if (entities.find(name) != entities.end()) {
				return entities[name];
			}

			auto slash = name.find_last_of('/');
			if (slash == std::string::npos) {
				std::printf("ecs: invalid name");
				std::abort();
			}
			
			auto parent_name = slash == 0 ? "/" : name.substr(0, slash);
			auto parent = create(parent_name);

			entities[name] = hierarchy->registry.create();
			fathers[entities[name]] = entities[parent_name];
			sons[entities[parent_name]].push_back(entities[name]);
			return entities[name];
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
				std::printf("ecs: invalid key");
				std::abort();
			}
		}
	};

	Hierarchy::Hierarchy() noexcept {
		impl->hierarchy = this;
		impl->plant();
	}

	auto Hierarchy::create(std::string const& name) noexcept -> Entity {
		return impl->create(name);
	}

	auto Hierarchy::entity(std::string const& name) const noexcept -> Entity {
		return impl->fetch(impl->entities, name);
	}

	auto Hierarchy::parent(Entity entity) const noexcept -> Entity {
		return impl->fetch(impl->fathers, entity);
	}

	auto Hierarchy::children(Entity entity) const noexcept -> std::vector<Entity> const& {
		return impl->fetch(impl->sons, entity);
	}

	auto Hierarchy::update() noexcept -> void {
		for (auto& stage: stages) {
			stage->update(*this);
		}
	}
}
