#pragma once
#include <metatron/scene/ecs/daemon.hpp>
#include <metatron/core/stl/capsule.hpp>
#include <vector>

namespace mtt::ecs {
	struct Daemon;
	struct Hierarchy;

	struct Stage final: stl::capsule<Stage> {
		struct Impl;
		Stage(std::vector<mut<Daemon>>&& daemons) noexcept;

		auto update(Hierarchy const&) noexcept -> void;
	};
}
