#pragma once
#include <metatron/core/math/vector.hpp>
#include <algorithm>

namespace mtt::spectra {
	template<usize n>
	struct Discrete_Spectrum final {
		Discrete_Spectrum(std::array<f32, n>&& lambda, std::array<f32, n>&& data) noexcept
		: lambda(std::move(lambda)), data(std::move(data)) {}

		Discrete_Spectrum(std::array<math::Vector<f32, 2>, n>&& interleaved) noexcept {
			for (auto i = 0uz; i < n; i++) {
				lambda[i] = interleaved[i][0];
				data[i] = interleaved[i][1];
			}
		}

		auto operator()(f32 lambda) const noexcept -> f32 {
			if (lambda < this->lambda.front() || lambda > this->lambda.back()) {
				return 0.f;
			}

			auto idx = std::max(0uz, std::min(
				n - 2,
				std::lower_bound(
					this->lambda.begin(),
					this->lambda.end(),
					lambda
				) - this->lambda.begin() - 1uz
			));
			auto alpha = (lambda - this->lambda[idx]) / (this->lambda[idx + 1] - this->lambda[idx]);
			return math::lerp(data[idx], data[idx + 1], alpha);
		}

	private:
		std::array<f32, n> lambda;
		std::array<f32, n> data;
	};
}
