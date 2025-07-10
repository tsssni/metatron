#pragma once
#include <metatron/core/math/ray.hpp>
#include <metatron/core/math/bounding-box.hpp>
#include <metatron/resource/eval/context.hpp>
#include <optional>

namespace mtt::shape {
	struct Interaction final {
		math::Vector<f32, 3> p;
		math::Vector<f32, 3> n;
		math::Vector<f32, 3> tn;
		math::Vector<f32, 3> bn;
		math::Vector<f32, 2> uv;
		f32 t;

		math::Vector<f32, 3> dpdu;
		math::Vector<f32, 3> dpdv;
		math::Vector<f32, 3> dndu;
		math::Vector<f32, 3> dndv;

		f32 pdf;
	};

	MTT_POLY_METHOD(shape_size, size);
	MTT_POLY_METHOD(shape_bounding_box, bounding_box);
	MTT_POLY_METHOD(shape_sample, sample);

	struct Shape final: pro::facade_builder
	::add_convention<shape_size, auto () const noexcept -> usize>
	::add_convention<shape_bounding_box, auto (
		math::Matrix<f32, 4, 4> const& t,
		usize idx
	) const noexcept -> math::Bounding_Box>
	::add_convention<pro::operator_dispatch<"()">, auto (
		math::Ray const& r,
		math::Vector<f32, 3> const& np,
		usize idx
	) const noexcept -> std::optional<Interaction>>
	::add_convention<shape_sample, auto (
		eval::Context const& ctx,
		math::Vector<f32, 2> const& u,
		usize idx
	) const noexcept -> std::optional<Interaction>>
	::support<pro::skills::as_view>
	::build {};
}
