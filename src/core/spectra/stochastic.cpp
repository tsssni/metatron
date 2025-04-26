#include <metatron/core/spectra/stochastic.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/arithmetic.hpp>

namespace metatron::spectra {
	Stochastic_Spectrum::Stochastic_Spectrum(usize n, f32 u, f32 v) {
		for (auto i = 0; i < n; i++) {
			auto ui = std::fmod(u + i / float(n), 1.f);
			lambda.emplace_back(std::lerp(visible_lambda[0], visible_lambda[1], ui));
		}
		value.resize(n, v);
		pdf.resize(n, 1.f / (visible_lambda[1] - visible_lambda[0]));
	}

	auto Stochastic_Spectrum::operator()(f32 lambda) const -> f32 {
		for (auto i = 0uz; i < this->lambda.size(); i++) {
			if (lambda == this->lambda[i]) {
				return value[i];
			}
		}
		std::printf("no matched lambda in stochastic spectrum\n");
		std::abort();
	}

	auto Stochastic_Spectrum::operator()(Spectrum const& spectrum) const -> f32 {
		auto f = 0.f;
		for (auto i = 0uz; i < lambda.size(); i++) {
			f += value[i] * spectrum(lambda[i]) / pdf[i];
		}
		return f / lambda.size();
	}

	auto Stochastic_Spectrum::operator&(Spectrum const& spectrum) const -> Stochastic_Spectrum {
		auto spec = *this;
		for (auto i = 0uz; i < lambda.size(); i++) {
			spec.value[i] = spectrum(lambda[i]);
		}
		return spec;
	}

	auto Stochastic_Spectrum::operator+(Spectrum const& spectrum) const -> Stochastic_Spectrum {
		auto spec = *this;
		spec += spectrum;
		return spec;
	}
	
	auto Stochastic_Spectrum::operator+=(Spectrum const& spectrum) -> Stochastic_Spectrum& {
		for (auto i = 0uz; i < lambda.size(); i++) {
			value[i] += spectrum(lambda[i]);
		}
		return *this;
	}

	auto Stochastic_Spectrum::operator-(Spectrum const& spectrum) const -> Stochastic_Spectrum {
		auto spec = *this;
		spec -= spectrum;
		return spec;
	}
	
	auto Stochastic_Spectrum::operator-=(Spectrum const& spectrum) -> Stochastic_Spectrum& {
		for (auto i = 0uz; i < lambda.size(); i++) {
			value[i] -= spectrum(lambda[i]);
		}
		return *this;
	}

	auto Stochastic_Spectrum::operator*(Spectrum const& spectrum) const -> Stochastic_Spectrum {
		auto spec = *this;
		spec *= spectrum;
		return spec;
	}
	
	auto Stochastic_Spectrum::operator*=(Spectrum const& spectrum) -> Stochastic_Spectrum& {
		for (auto i = 0uz; i < lambda.size(); i++) {
			value[i] *= spectrum(lambda[i]);
		}
		return *this;
	}

	auto Stochastic_Spectrum::operator/(Spectrum const& spectrum) const -> Stochastic_Spectrum {
		auto spec = *this;
		spec /= spectrum;
		return spec;
	}
	
	auto Stochastic_Spectrum::operator/=(Spectrum const& spectrum) -> Stochastic_Spectrum& {
		for (auto i = 0uz; i < lambda.size(); i++) {
			value[i] /= spectrum(lambda[i]);
		}
		return *this;
	}

	auto Stochastic_Spectrum::operator=(f32 s) -> Stochastic_Spectrum& {
		for (auto& v: value) {
			v = s;
		}
		return *this;
	}

	auto Stochastic_Spectrum::operator+(f32 s) const -> Stochastic_Spectrum {
		auto spec = *this;
		spec += s;
		return spec;
	};

	auto Stochastic_Spectrum::operator+=(f32 s) -> Stochastic_Spectrum& {
		for (auto& v: value) {
			v += s;
		}
		return *this;
	};

	auto Stochastic_Spectrum::operator-(f32 s) const -> Stochastic_Spectrum {
		auto spec = *this;
		spec -= s;
		return spec;
	};

	auto Stochastic_Spectrum::operator-=(f32 s) -> Stochastic_Spectrum& {
		for (auto& v: value) {
			v -= s;
		}
		return *this;
	};

	auto Stochastic_Spectrum::operator*(f32 s) const -> Stochastic_Spectrum {
		auto spec = *this;
		spec *= s;
		return spec;
	};

	auto Stochastic_Spectrum::operator*=(f32 s) -> Stochastic_Spectrum& {
		for (auto& v: value) {
			v *= s;
		}
		return *this;
	};

	auto Stochastic_Spectrum::operator/(f32 s) const -> Stochastic_Spectrum {
		auto spec = *this;
		spec /= s;
		return spec;
	};

	auto Stochastic_Spectrum::operator/=(f32 s) -> Stochastic_Spectrum& {
		for (auto& v: value) {
			v = math::guarded_div(v, s);
		}
		return *this;
	};

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
		for (auto& v: spec.value) {
			v = math::guarded_div(s, v);
		}
		return spec;
	}

	auto min(Stochastic_Spectrum const& spectrum) -> f32 {
		auto minv = math::maxv<f32>;
		for (auto& v: spectrum.value) {
			minv = std::min(minv, v);
		}
		return minv;
	}

	auto max(Stochastic_Spectrum const& spectrum) -> f32 {
		auto maxv = math::minv<f32>;
		for (auto& v: spectrum.value) {
			maxv = std::max(maxv, v);
		}
		return maxv;
	}


	auto avg(Stochastic_Spectrum const& spectrum) -> f32 {
		auto n = spectrum.lambda.size();
		auto avgv = 0.f;
		for (auto& v: spectrum.value) {
			avgv += math::guarded_div(v, n);
		}
		return avgv;
	}

	auto clear(Stochastic_Spectrum& spectrum, f32 cv) -> void {
		for (auto& v: spectrum.value) {
			v = cv;
		}
	}
}
