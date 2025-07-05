#pragma once
#include <metatron/scene/ecs/entity.hpp>

namespace mtt::ecs {
	struct Hierarchy;

	struct Daemon {
		auto virtual trigger(Hierarchy& hierarychy) -> void = 0;
		auto virtual attach(Hierarchy const& hierarychy, Entity entity) -> void = 0;
		auto virtual mutate(Hierarchy const& hierarychy, Entity entity) -> void = 0;
		auto virtual detach(Hierarchy const& hierarychy, Entity entity) -> void = 0;
	};
}
