#pragma once
#include <metatron/core/math/vector.hpp>
#include <unordered_map>

namespace mtt::spectra {
	auto constexpr visible_lambda = math::Vector<f32, 2>{360.f, 830.f};

	struct Spectrum final: pro::facade_builder
	::add_convention<pro::operator_dispatch<"()">, auto (f32) const noexcept -> f32>
	::add_skill<pro::skills::as_view>
	::build {
		std::unordered_map<std::string, view<Spectrum>> static spectra;
	};

	auto inline operator|(view<Spectrum> x, view<Spectrum> y) noexcept -> f32 {
		auto integral = 0.f;
		for (auto lambda = visible_lambda[0]; lambda <= visible_lambda[1]; lambda++) {
			integral += (*x)(lambda) * (*y)(lambda);
		}
		return integral;
	}

	auto inline operator~(view<Spectrum> s) noexcept -> math::Vector<f32, 3> {
		return math::Vector<f32, 3>{
			Spectrum::spectra["CIE-X"] | s,
			Spectrum::spectra["CIE-Y"] | s,
			Spectrum::spectra["CIE-Z"] | s,
		};
	}
}
