#pragma once
#include <metatron/core/math/vector.hpp>

namespace mtt::math {
	namespace filter {
		struct Interaction final {
			Vector<f32, 2> p;
			f32 weight;
			f32 pdf;
		};
	}

	MTT_POLY_METHOD(filter_sampler, sample);

	struct Filter final: pro::facade_builder
	::add_convention<pro::operator_dispatch<"()">, auto (
		Vector<f32, 2> const& p
	) const noexcept -> f32>
	::add_convention<filter_sampler, auto (
		Vector<f32, 2> const& u
	) -> std::optional<filter::Interaction>>
	::support<pro::skills::as_view>
	::build {};
}
