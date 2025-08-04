#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <metatron/render/emitter/emitter.hpp>
#include <metatron/render/accel/accel.hpp>

namespace mtt::daemon {
	struct Tracer_Daemon final {
		ecs::Entity tracer;
		auto init() noexcept -> void;
		auto update() noexcept -> void;
		auto render(std::string_view path) noexcept -> void;
	};
}
