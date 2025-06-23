#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <cstdio>
#include <cstdlib>

namespace metatron::spectra {
	Stochastic_Spectrum::Stochastic_Spectrum(f32 u, f32 v) {
		lambda = math::foreach([&](f32 l, usize i) {
			auto ui = std::fmod(u + i / f32(stochastic_samples), 1.f);
			return math::lerp(visible_lambda[0], visible_lambda[1], ui);
		}, lambda);
		value = math::Vector<f32, stochastic_samples>{v};
		pdf = math::Vector<f32, stochastic_samples>{1.f / (visible_lambda[1] - visible_lambda[0])};
	}

	auto Stochastic_Spectrum::operator()(f32 lambda) const -> f32 {
		for (auto i = 0uz; i < stochastic_samples; i++) {
			if (lambda == this->lambda[i]) {
				return value[i];
			}
		}
		std::printf("no matched lambda in stochastic spectrum\n");
		std::abort();
	}

	auto Stochastic_Spectrum::operator()(Spectrum const& spectrum) const -> f32 {
		return math::sum(math::foreach([&](f32 lambda, f32 value, f32 pdf, usize i) {
			return value * spectrum(lambda) / pdf;
		}, lambda, value, pdf)) / stochastic_samples;
	}

	auto Stochastic_Spectrum::operator&(Spectrum const& spectrum) const -> Stochastic_Spectrum {
		auto spec = *this;
		spec.value = math::foreach([&](f32 lambda, usize i) {
			return spectrum(lambda);
		}, lambda);
		return spec;
	}

	auto Stochastic_Spectrum::operator+(Spectrum const& spectrum) const -> Stochastic_Spectrum {
		auto spec = *this;
		spec += spectrum;
		return spec;
	}
	
	auto Stochastic_Spectrum::operator+=(Spectrum const& spectrum) -> Stochastic_Spectrum& {
		value += math::foreach([&](f32 lambda, usize i) {
			return spectrum(lambda);
		}, lambda);
		return *this;
	}

	auto Stochastic_Spectrum::operator-(Spectrum const& spectrum) const -> Stochastic_Spectrum {
		auto spec = *this;
		spec -= spectrum;
		return spec;
	}
	
	auto Stochastic_Spectrum::operator-=(Spectrum const& spectrum) -> Stochastic_Spectrum& {
		value -= math::foreach([&](f32 lambda, usize i) {
			return spectrum(lambda);
		}, lambda);
		return *this;
	}

	auto Stochastic_Spectrum::operator*(Spectrum const& spectrum) const -> Stochastic_Spectrum {
		auto spec = *this;
		spec *= spectrum;
		return spec;
	}
	
	auto Stochastic_Spectrum::operator*=(Spectrum const& spectrum) -> Stochastic_Spectrum& {
		value *= math::foreach([&](f32 lambda, usize i) {
			return spectrum(lambda);
		}, lambda);
		return *this;
	}

	auto Stochastic_Spectrum::operator/(Spectrum const& spectrum) const -> Stochastic_Spectrum {
		auto spec = *this;
		spec /= spectrum;
		return spec;
	}
	
	auto Stochastic_Spectrum::operator/=(Spectrum const& spectrum) -> Stochastic_Spectrum& {
		value = math::foreach([&](f32 value, f32 lambda, usize i) {
			return math::guarded_div(value, spectrum(lambda));
		}, value, lambda);
		return *this;
	}

	auto Stochastic_Spectrum::operator=(f32 s) -> Stochastic_Spectrum& {
		value = math::Vector<f32, stochastic_samples>{s};
		return *this;
	}

	auto Stochastic_Spectrum::operator+(f32 s) const -> Stochastic_Spectrum {
		auto spec = *this;
		spec += s;
		return spec;
	};

	auto Stochastic_Spectrum::operator+=(f32 s) -> Stochastic_Spectrum& {
		value += math::Vector<f32, stochastic_samples>{s};
		return *this;
	};

	auto Stochastic_Spectrum::operator-(f32 s) const -> Stochastic_Spectrum {
		auto spec = *this;
		spec -= s;
		return spec;
	};

	auto Stochastic_Spectrum::operator-=(f32 s) -> Stochastic_Spectrum& {
		value -= math::Vector<f32, stochastic_samples>{s};
		return *this;
	};

	auto Stochastic_Spectrum::operator*(f32 s) const -> Stochastic_Spectrum {
		auto spec = *this;
		spec *= s;
		return spec;
	};

	auto Stochastic_Spectrum::operator*=(f32 s) -> Stochastic_Spectrum& {
		value *= math::Vector<f32, stochastic_samples>{s};
		return *this;
	};

	auto Stochastic_Spectrum::operator/(f32 s) const -> Stochastic_Spectrum {
		auto spec = *this;
		spec /= s;
		return spec;
	};

	auto Stochastic_Spectrum::operator/=(f32 s) -> Stochastic_Spectrum& {
		value /= math::Vector<f32, stochastic_samples>{s};
		return *this;
	};

	Stochastic_Spectrum::operator bool() const {
		return math::any([](f32 x, usize i) { return x > 0.f; }, value);
	}

	auto operator+(f32 s, Stochastic_Spectrum const& spectrum) -> Stochastic_Spectrum {
		return spectrum + s;
	}

	auto operator-(f32 s, Stochastic_Spectrum const& spectrum) -> Stochastic_Spectrum {
		return spectrum - s;
	}

	auto operator*(f32 s, Stochastic_Spectrum const& spectrum) -> Stochastic_Spectrum {
		return spectrum * s;
	}

	auto operator/(f32 s, Stochastic_Spectrum const& spectrum) -> Stochastic_Spectrum {
		auto spec = spectrum;
		spec.value = math::foreach([&](f32 v, usize i) {
			return math::guarded_div(s, v);
		}, spectrum.value);
		return spec;
	}

	auto min(Stochastic_Spectrum const& spectrum) -> f32 {
		return math::min(spectrum.value);
	}

	auto max(Stochastic_Spectrum const& spectrum) -> f32 {
		return math::max(spectrum.value);
	}

	auto avg(Stochastic_Spectrum const& spectrum) -> f32 {
		return math::sum(spectrum.value / stochastic_samples);
	}

	auto constant(Stochastic_Spectrum const& spectrum) -> bool {
		return math::all([&](f32 x, usize i) { return x == spectrum.value[0]; }, spectrum.value);
	}

	auto degrade(Stochastic_Spectrum& spectrum) -> void {
		spectrum.value = math::Vector<f32, stochastic_samples>{spectrum.value[0]};
		spectrum.lambda = math::Vector<f32, stochastic_samples>{visible_lambda[0]};
		spectrum.pdf = math::Vector<f32, stochastic_samples>{math::sum(spectrum.pdf)};
	}
}
