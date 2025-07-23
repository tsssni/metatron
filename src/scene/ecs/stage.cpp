#include <metatron/scene/ecs/stage.hpp>
#include <metatron/scene/ecs/daemon.hpp>
#include <metatron/scene/ecs/hierarchy.hpp>

namespace mtt::ecs {
	auto Stage::update(Hierarchy const& hierarchy) noexcept -> void {
		for (auto& daemon : daemons) {
			daemon->update(hierarchy);
		}
	}
}
