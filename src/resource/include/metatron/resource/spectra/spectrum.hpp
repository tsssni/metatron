#pragma once
#include <metatron/core/math/vector.hpp>

namespace mtt::spectra {
	auto constexpr visible_lambda = math::Vector<f32, 2>{360.f, 830.f};

	struct Spectrum: pro::facade_builder
					 ::add_convention<pro::operator_dispatch<"()">, f32(f32) const>
					 ::support<pro::skills::as_view>
					 ::build {

		pro::proxy<Spectrum> static one;
		pro::proxy<Spectrum> static zero;

		pro::proxy<Spectrum> static CIE_X;
		pro::proxy<Spectrum> static CIE_Y;
		pro::proxy<Spectrum> static CIE_Z;
		pro::proxy<Spectrum> static CIE_D65;

		pro::proxy<Spectrum> static Au_eta;
		pro::proxy<Spectrum> static Au_k;

		auto static initialize() -> void;
	};

	auto inline operator|(pro::proxy_view<Spectrum> x, pro::proxy_view<Spectrum> y) -> f32 {
		auto integral = 0.f;
		for (auto lambda = visible_lambda[0]; lambda <= visible_lambda[1]; lambda++) {
			integral += (*x)(lambda) * (*y)(lambda);
		}
		return integral;
	}

	auto inline operator~(pro::proxy_view<Spectrum> s) -> math::Vector<f32, 3> {
		return {
			Spectrum::CIE_X | s,
			Spectrum::CIE_Y | s,
			Spectrum::CIE_Z | s
		};
	}
}
