#include <metatron/scene/daemon/shape.hpp>
#include <metatron/scene/compo/shape.hpp>
#include <metatron/resource/shape/shape.hpp>
#include <metatron/resource/shape/mesh.hpp>
#include <metatron/resource/shape/sphere.hpp>
#include <print>

namespace mtt::daemon {
	auto Shape_Daemon::update(ecs::Hierarchy& hierarchy) noexcept -> void {
		auto& registry = hierarchy.registry;

		auto shape_view = registry.view<ecs::Dirty_Mark<compo::Shape>>();
		for (auto entity: shape_view) {
			if (registry.any_of<poly<shape::Shape>>(entity)) {
				registry.erase<poly<shape::Shape>>(entity);
			}
			if (!registry.any_of<compo::Shape>(entity)) {
				continue;
			}

			auto& shape = registry.get<compo::Shape>(entity);
			std::visit([&](auto&& compo) -> void {
				using T = std::decay_t<decltype(compo)>;
				if constexpr (std::is_same_v<T, compo::Mesh>) {
					registry.emplace<poly<shape::Shape>>(
						entity,
						make_poly<shape::Shape>(shape::Mesh::from_path(compo.path))
					);
				} else if constexpr (std::is_same_v<T, compo::Sphere>) {
					registry.emplace<poly<shape::Shape>>(
						entity,
						make_poly<shape::Shape>(shape::Sphere{})
					);
				}
			}, shape);
		}
		registry.clear<ecs::Dirty_Mark<compo::Shape>>();
	}
}
