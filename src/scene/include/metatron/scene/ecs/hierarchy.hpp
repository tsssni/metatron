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

		template<typename T, view<char> S>
		auto static serde() noexcept -> void {
			auto fr = [](ecs::Entity e, std::string const& s) -> void {
				auto d = T{};
				if(auto e = glz::read_json<T>(d, s); !e) {
					std::println(glz::format_error(e, s));
					std::abort();
				} else {
					ecs::Hierarchy::instance->attach<T>(e, d);
				}
			};
			auto fw = []() -> std::vector<serde::json> {
				auto v = std::vector<serde::json>{};
				for(auto&& [e, d]: ecs::Hierarchy::instance->registry.view<T>()) {
					auto s = glz::write_json(d).value_or("error");
					if (s == "error") {
						std::println(
							"Failed to serialize component {} on {}",
							S, ecs::Hierarchy::instance->path(e)
						);
						std::abort();
					}
					v.push_back({e, S, glz::write_json(d).value()});
				}
				return v;
			};
			ecs::Hierarchy::instance->enable(S, std::move(fr), std::move(fw));
		}
		auto enable(
			std::string_view type,
			std::function<void(ecs::Entity e, std::string const& s)> fr,
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
}
