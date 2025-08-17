#include <metatron/scene/daemon/shape.hpp>
#include <metatron/scene/compo/shape.hpp>
#include <metatron/scene/ecs/hierarchy.hpp>
#include <metatron/resource/shape/shape.hpp>
#include <metatron/resource/shape/mesh.hpp>
#include <metatron/resource/shape/sphere.hpp>

namespace mtt::daemon {
	template<char const* str>
	auto f() -> void {}

	auto Shape_Daemon::init() noexcept -> void {
		MTT_SERDE(Shape);
		MTT_SERDE(Shape_Instance);
	}

	auto Shape_Daemon::update() noexcept -> void {
		auto& hierarchy = *ecs::Hierarchy::instance;
		auto& registry = hierarchy.registry;
		auto shape_view = registry.view<ecs::Dirty_Mark<compo::Shape>>();
		for (auto entity: shape_view) {
			registry.remove<poly<shape::Shape>>(entity);
			if (!registry.any_of<compo::Shape>(entity)) {
				continue;
			}
			auto& shape = registry.get<compo::Shape>(entity);

			registry.emplace<poly<shape::Shape>>(entity,
			std::visit([&](auto&& compo) -> poly<shape::Shape> {
				using T = std::decay_t<decltype(compo)>;
				if constexpr (std::is_same_v<T, compo::Sphere>) {
					return make_poly<shape::Shape, shape::Sphere>();
				} else if constexpr (std::is_same_v<T, compo::Mesh>) {
					auto const& wd = registry.get<ecs::Working_Directory>(hierarchy.root());
					return make_poly<shape::Shape, shape::Mesh>(
						shape::Mesh::from_path(wd.path + compo.path)
					);
				}
			},shape));
		}
		registry.clear<ecs::Dirty_Mark<compo::Shape>>();
	}
}
