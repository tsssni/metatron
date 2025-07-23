#pragma once
#include <metatron/scene/ecs/entity.hpp>

namespace mtt::ecs {
	MTT_POLY_METHOD(daemon_update, update);

	struct Daemon final: pro::facade_builder
	::add_convention<daemon_update, auto (
		Registry& registry
	) noexcept -> void>
	::support<pro::skills::as_view>
	::build {};
}
