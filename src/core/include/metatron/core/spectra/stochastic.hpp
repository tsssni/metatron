#pragma once
#include <metatron/core/spectra/spectrum.hpp>
#include <vector>

namespace metatron::spectra {
	auto constexpr stochastic_samples = 4uz;

	struct Stochastic_Spectrum final: Spectrum {
		std::vector<f32> lambda;
		std::vector<f32> pdf;
		std::vector<f32> value;

		Stochastic_Spectrum() = default;
		Stochastic_Spectrum(usize n, f32 u, f32 v = 0.f);

		auto operator()(f32 lambda) const -> f32;
		auto operator()(Spectrum const& spectrum) const -> f32;
		auto operator&(Spectrum const& spectrum) const -> Stochastic_Spectrum;

		auto operator+(Spectrum const& spectrum) const -> Stochastic_Spectrum;
		auto operator+=(Spectrum const& spectrum) -> Stochastic_Spectrum&;
		auto operator-(Spectrum const& spectrum) const -> Stochastic_Spectrum;
		auto operator-=(Spectrum const& spectrum) -> Stochastic_Spectrum&;
		auto operator*(Spectrum const& spectrum) const -> Stochastic_Spectrum;
		auto operator*=(Spectrum const& spectrum) -> Stochastic_Spectrum&;
		auto operator/(Spectrum const& spectrum) const -> Stochastic_Spectrum;
		auto operator/=(Spectrum const& spectrum) -> Stochastic_Spectrum&;

		auto operator+(f32 s) const -> Stochastic_Spectrum;
		auto operator+=(f32 s) -> Stochastic_Spectrum&;
		auto operator-(f32 s) const -> Stochastic_Spectrum;
		auto operator-=(f32 s) -> Stochastic_Spectrum&;
		auto operator*(f32 s) const -> Stochastic_Spectrum;
		auto operator*=(f32 s) -> Stochastic_Spectrum&;
		auto operator/(f32 s) const -> Stochastic_Spectrum;
		auto operator/=(f32 s) -> Stochastic_Spectrum&;
	};

	auto operator+(f32 s, Stochastic_Spectrum const& spectrum) -> Stochastic_Spectrum;
	auto operator-(f32 s, Stochastic_Spectrum const& spectrum) -> Stochastic_Spectrum;
	auto operator*(f32 s, Stochastic_Spectrum const& spectrum) -> Stochastic_Spectrum;
	auto operator/(f32 s, Stochastic_Spectrum const& spectrum) -> Stochastic_Spectrum;

	auto min(Stochastic_Spectrum const& spectrum) -> f32;
	auto max(Stochastic_Spectrum const& spectrum) -> f32;
	auto avg(Stochastic_Spectrum const& spectrum) -> f32;
	auto clear(Stochastic_Spectrum& spectrum, f32 v = 0.f) -> void;
}
