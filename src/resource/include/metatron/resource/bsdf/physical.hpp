#pragma once
#include <metatron/resource/bsdf/bsdf.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::bsdf {
	struct Physical_Bsdf final: stl::capsule<Physical_Bsdf> {
		struct Impl;
		Physical_Bsdf() noexcept;

		auto operator()(
			math::Vector<f32, 3> const& wo,
			math::Vector<f32, 3> const& wi
		) const noexcept -> std::optional<Interaction>;
		auto sample(
			eval::Context const& ctx,
			math::Vector<f32, 3> const& u
		) const noexcept -> std::optional<Interaction>;
		auto configure(Attribute const& attr) noexcept -> void;
		auto flags() const noexcept -> Flags;
		auto degrade() noexcept -> bool;
	};
}
