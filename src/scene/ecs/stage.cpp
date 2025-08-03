#include <metatron/scene/ecs/stage.hpp>
#include <metatron/scene/ecs/daemon.hpp>
#include <metatron/scene/ecs/hierarchy.hpp>

namespace mtt::ecs {
	auto Stage::init() noexcept -> void {
		for (auto& daemon : daemons) {
			daemon->init();
		}
	}

	auto Stage::update() noexcept -> void {
		for (auto& daemon : daemons) {
			daemon->update();
		}
	}
}
