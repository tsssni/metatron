#include <metatron/scene/ecs/stage.hpp>
#include <metatron/scene/ecs/daemon.hpp>
#include <metatron/scene/ecs/hierarchy.hpp>

namespace mtt::ecs {
	struct Stage::Impl final {
		std::vector<mut<Daemon>> daemons;

		auto update(Hierarchy const& hierarchy) const noexcept -> void {
			for (auto daemon : daemons) {
				daemon->update(hierarchy);
			}
		}
	};

	Stage::Stage(std::vector<mut<Daemon>>&& daemons) noexcept {
		impl->daemons = std::move(daemons);
	}

	auto Stage::update(Hierarchy const& hierarchy) noexcept -> void {
		impl->update(hierarchy);
	}
}
