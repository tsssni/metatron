#pragma once

namespace mtt::ecs {
	struct Hierarchy;

	MTT_POLY_METHOD(daemon_update, update);

	struct Daemon final: pro::facade_builder
	::add_convention<daemon_update, auto (
		Hierarchy const&
	) noexcept -> void>
	::support<pro::skills::as_view>
	::build {};
}
