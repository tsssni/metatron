#pragma once

namespace mtt::ecs {
	struct Hierarchy;

	MTT_POLY_METHOD(daemon_init, init);
	MTT_POLY_METHOD(daemon_update, update);

	struct Daemon final: pro::facade_builder
	::add_convention<daemon_init, auto () noexcept -> void>
	::add_convention<daemon_update, auto () noexcept -> void>
	::support_view
	::build {};
}
