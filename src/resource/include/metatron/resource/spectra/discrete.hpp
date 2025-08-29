#pragma once
#include <metatron/core/math/vector.hpp>
#include <vector>

namespace mtt::spectra {
	struct Discrete_Spectrum final {
		Discrete_Spectrum(std::vector<f32>&& lambda, std::vector<f32>&& data) noexcept;
		Discrete_Spectrum(std::vector<math::Vector<f32, 2>>&& interleaved) noexcept;

		auto operator()(f32 lambda) const noexcept -> f32;

	private:
		std::vector<f32> lambda;
		std::vector<f32> data;
	};
}
