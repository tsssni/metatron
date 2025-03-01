#include <metatron/render/spectra/spectrum.hpp>

namespace metatron::spectra {
	Spectrum::~Spectrum() {};

	Stochastic_Spectrum::Stochastic_Spectrum(usize n, math::Sampler const& sampler) {
		lambda.resize(n, 1.f);
		pdf.resize(n, 1.f);
		value.resize(n, 1.f);
	}

	auto Stochastic_Spectrum::operator()(f32 lambda) -> f32& {
		for (auto i = 0uz; i < this->lambda.size(); i++) {
			if (lambda == this->lambda[i]) {
				return value[i];
			}
		}
		std::abort();
	}

	auto Stochastic_Spectrum::operator()(f32 lambda) const -> f32 const& {
		return (*(const_cast<Stochastic_Spectrum*>(this)))(lambda);
	}

	auto Stochastic_Spectrum::operator*(Spectrum const& spectrum) const -> f32 {
		auto f = 0.f;
		for (auto i = 0uz; i < lambda.size(); i++) {
			f += value[i] * spectrum(lambda[i]) / pdf[i];
		}
		return f / lambda.size();
	}
}
