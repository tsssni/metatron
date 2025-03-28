#pragma once
#include <metatron/core/spectra/spectrum.hpp>
#include <algorithm>

namespace metatron::spectra {
	template<usize n>
	struct Discrete_Spectrum final: Spectrum {
		Discrete_Spectrum(std::array<f32, n>&& lambda, std::array<f32, n>&& data)
			: lambda(std::move(lambda)), data(std::move(data)) {}

		Discrete_Spectrum(std::array<f32, 2uz * n>&& interleaved) {
			for (auto i = 0uz; i < n; i++) {
				lambda[i] = interleaved[i * 2uz];
				data[i] = interleaved[i * 2uz + 1uz];
			}
		}

		auto operator()(f32 lambda) const -> f32 {
			if (lambda < this->lambda.front() || lambda > this->lambda.back()) {
				return 0.f;
			}

			auto idx = std::lower_bound(this->lambda.begin(), this->lambda.end(), lambda) - this->lambda.begin();
			if (idx == data.size() - 1) {
				return data.back();
			}

			auto alpha = (lambda - this->lambda[idx]) / (this->lambda[idx + 1] - this->lambda[idx]);
			return std::lerp(data[idx], data[idx + 1], alpha);
		}

	private:
		std::array<f32, n> lambda;
		std::array<f32, n> data;
	};
}
