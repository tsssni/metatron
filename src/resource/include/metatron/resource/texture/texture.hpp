#pragma once
#include <metatron/resource/shape/sphere.hpp>
#include <metatron/resource/eval/context.hpp>

namespace mtt::texture {
	struct Coordinate final {
		math::Vector<f32, 2> uv{};
		f32 dudx{0.f};
		f32 dudy{0.f};
		f32 dvdx{0.f};
		f32 dvdy{0.f};
	};

	MTT_POLY_METHOD(texture_sample, sample);

	template<typename T>
	struct Texture final: pro::facade_builder
	::add_convention<texture_sample, auto (
		eval::Context const& ctx,
		Coordinate const& coord
	) const noexcept -> T>
	::support_view
	::build {};

	auto grad(
		math::Ray_Differential const& diff,
		shape::Interaction const& intr
	) noexcept -> std::optional<texture::Coordinate>;
}
