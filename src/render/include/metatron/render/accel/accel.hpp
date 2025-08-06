#pragma once
#include <metatron/render/emitter/emitter.hpp>
#include <metatron/resource/shape/shape.hpp>
#include <metatron/resource/material/material.hpp>
#include <metatron/resource/media/medium.hpp>
#include <metatron/core/math/bounding-box.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/math/transform.hpp>

namespace mtt::accel {
	struct Divider final {
		view<shape::Shape> shape{};
		view<media::Medium> medium;
		view<light::Light> light{};
		material::Material const* material{};
		math::Transform const* local_to_world{};
		math::Transform const* medium_to_world{};
		usize primitive{0uz};
	};

	struct Interaction final {
		Divider const* divider{nullptr};
		std::optional<shape::Interaction> intr_opt;
	};

	struct Acceleration final: pro::facade_builder
	::add_convention<pro::operator_dispatch<"()">, auto (
		math::Ray const& r,
		math::Vector<f32, 3> const& np
	) const noexcept -> std::optional<Interaction>>
	::add_skill<pro::skills::as_view>
	::build {};
}
