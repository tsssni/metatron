#include <metatron/core/spectra/stochastic.hpp>
#include <metatron/core/math/vector.hpp>

namespace metatron::spectra {
	Stochastic_Spectrum::Stochastic_Spectrum(usize n, f32 u) {
		auto constexpr lambda_min = 380.f;
		auto constexpr lambda_max = 780.f;
		for (auto i = 0; i < n; i++) {
			auto ui = std::fmod(u + 1.f / float(n), 1.f);
			lambda.emplace_back(std::lerp(380.f, 780.f, ui));
			value.emplace_back(1.f);
			pdf.emplace_back(1.f / (lambda_max - lambda_min));
		}
	}

	auto Stochastic_Spectrum::operator()(f32 lambda) const -> f32 {
		for (auto i = 0uz; i < this->lambda.size(); i++) {
			if (lambda == this->lambda[i]) {
				return value[i];
			}
		}
		std::abort();
	}

	auto Stochastic_Spectrum::operator()(Spectrum const& spectrum) const -> f32 {
		auto f = 0.f;
		for (auto i = 0uz; i < lambda.size(); i++) {
			f += value[i] * spectrum(lambda[i]) / pdf[i];
		}
		return f / lambda.size();
	}

	auto Stochastic_Spectrum::operator*(f32 s) -> Spectrum& {
		for (auto i = 0uz; i < lambda.size(); i++) {
			value[i] *= s;
		}
		return *this;
	};

	auto Stochastic_Spectrum::operator*(Spectrum const& spectrum) const -> Spectrum const& {
		for (auto i = 0uz; i < lambda.size(); i++) {
			value[i] *= spectrum(lambda[i]);
		}
		return *this;
	}
}
