#pragma once
#include <metatron/resource/spectra/spectrum.hpp>
#include <metatron/core/math/vector.hpp>

namespace metatron::spectra {
	auto constexpr stochastic_samples = 4uz;

	struct Stochastic_Spectrum final: Spectrum {
		math::Vector<f32, stochastic_samples> lambda{};
		math::Vector<f32, stochastic_samples> pdf{};
		math::Vector<f32, stochastic_samples> value{};

		Stochastic_Spectrum() = default;
		Stochastic_Spectrum(f32 u, f32 v = 0.f);

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

		auto operator=(f32 s) -> Stochastic_Spectrum&;
		auto operator+(f32 s) const -> Stochastic_Spectrum;
		auto operator+=(f32 s) -> Stochastic_Spectrum&;
		auto operator-(f32 s) const -> Stochastic_Spectrum;
		auto operator-=(f32 s) -> Stochastic_Spectrum&;
		auto operator*(f32 s) const -> Stochastic_Spectrum;
		auto operator*=(f32 s) -> Stochastic_Spectrum&;
		auto operator/(f32 s) const -> Stochastic_Spectrum;
		auto operator/=(f32 s) -> Stochastic_Spectrum&;

		explicit operator bool() const;
	};

	auto operator+(f32 s, Stochastic_Spectrum const& spectrum) -> Stochastic_Spectrum;
	auto operator-(f32 s, Stochastic_Spectrum const& spectrum) -> Stochastic_Spectrum;
	auto operator*(f32 s, Stochastic_Spectrum const& spectrum) -> Stochastic_Spectrum;
	auto operator/(f32 s, Stochastic_Spectrum const& spectrum) -> Stochastic_Spectrum;

	auto min(Stochastic_Spectrum const& spectrum) -> f32;
	auto max(Stochastic_Spectrum const& spectrum) -> f32;
	auto avg(Stochastic_Spectrum const& spectrum) -> f32;

	auto constant(Stochastic_Spectrum const& spectrum) -> bool;
	auto coherent(Stochastic_Spectrum const& spectrum) -> bool;
	auto degrade(Stochastic_Spectrum& spectrum) -> void;
}
