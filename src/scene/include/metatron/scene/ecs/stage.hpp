#pragma once
#include <metatron/scene/ecs/daemon.hpp>
#include <metatron/core/stl/capsule.hpp>
#include <vector>

namespace mtt::ecs {
	struct Hierarchy;

	struct Stage final: stl::capsule<Stage> {
		std::vector<poly<Daemon>> daemons;
		auto update(Hierarchy const&) noexcept -> void;
	};
}
