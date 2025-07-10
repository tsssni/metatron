#pragma once
#include <metatron/scene/ecs/entity.hpp>

namespace mtt::ecs {
	struct Hierarchy;

	MTT_POLY_METHOD(daemon_trigger, trigger);
	MTT_POLY_METHOD(daemon_attach, attach);
	MTT_POLY_METHOD(daemon_mutate, mutate);
	MTT_POLY_METHOD(daemon_detach, detach);

	struct Daemon final: pro::facade_builder
	::add_convention<daemon_trigger, auto (
		Hierarchy const& hierarchy
	) noexcept -> void>
	::add_convention<daemon_attach, auto (
		Hierarchy const& hierarchy, Entity entity
	) noexcept -> void>
	::add_convention<daemon_mutate, auto (
		Hierarchy const& hierarchy, Entity entity
	) noexcept -> void>
	::add_convention<daemon_detach, auto (
		Hierarchy const& hierarchy, Entity entity
	) noexcept -> void>
	::support<pro::skills::as_view>
	::build {};
}
