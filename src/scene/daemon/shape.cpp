#include <metatron/scene/daemon/shape.hpp>
#include <metatron/scene/compo/shape.hpp>
#include <metatron/scene/ecs/hierarchy.hpp>
#include <metatron/resource/shape/shape.hpp>
#include <metatron/resource/shape/mesh.hpp>
#include <metatron/resource/shape/sphere.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/optional.hpp>
#include <metatron/core/stl/print.hpp>

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

		auto view = registry.view<ecs::Dirty_Mark<compo::Shape>>()
		| std::views::filter([&](auto entity) {
			registry.remove<poly<shape::Shape>>(entity);
			return registry.any_of<compo::Shape>(entity);
		})
		| std::ranges::to<std::vector<ecs::Entity>>();

		auto mutex = std::mutex{};
		stl::scheduler::instance().sync_parallel(math::Vector<usize, 1>{view.size()}, [&](auto idx) {
			auto entity = view[idx[0]];
			auto& shape = registry.get<compo::Shape>(entity);

			auto s = std::visit([&](auto&& compo) -> poly<shape::Shape> {
				using T = std::decay_t<decltype(compo)>;
				if constexpr (std::is_same_v<T, compo::Sphere>) {
					return make_poly<shape::Shape, shape::Sphere>();
				} else if constexpr (std::is_same_v<T, compo::Mesh>) {
					MTT_OPT_OR_CALLBACK(path, stl::filesystem::instance().find(compo.path), {
						std::println("medium {} does not exist", compo.path);
						std::abort();
					});
					return make_poly<shape::Shape, shape::Mesh>(
						shape::Mesh::from_path(path)
					);
				}
			},shape);

			{
				auto lock = std::lock_guard(mutex);
				registry.emplace<poly<shape::Shape>>(entity, std::move(s));
			}
		});
		registry.clear<ecs::Dirty_Mark<compo::Shape>>();
	}
}
