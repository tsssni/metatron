#pragma once
#include <metatron/resource/spectra/spectrum.hpp>
#include <metatron/core/math/vector.hpp>

namespace mtt::spectra {
	auto constexpr stochastic_samples = 4uz;

	struct Stochastic_Spectrum final {
		math::Vector<f32, stochastic_samples> lambda{};
		math::Vector<f32, stochastic_samples> pdf{};
		math::Vector<f32, stochastic_samples> value{};

		Stochastic_Spectrum() noexcept = default;
		Stochastic_Spectrum(f32 u, f32 v = 0.f) noexcept;

		auto operator()(f32 lambda) const noexcept -> f32;
		auto operator()(view<Spectrum> spectrum) const noexcept -> f32;
		auto operator&(view<Spectrum> spectrum) const noexcept -> Stochastic_Spectrum;

		auto operator+(Stochastic_Spectrum const& spectrum) const noexcept -> Stochastic_Spectrum;
		auto operator+=(Stochastic_Spectrum const& spectrum) noexcept -> Stochastic_Spectrum&;
		auto operator-(Stochastic_Spectrum const& spectrum) const noexcept -> Stochastic_Spectrum;
		auto operator-=(Stochastic_Spectrum const& spectrum) noexcept -> Stochastic_Spectrum&;
		auto operator*(Stochastic_Spectrum const& spectrum) const noexcept -> Stochastic_Spectrum;
		auto operator*=(Stochastic_Spectrum const& spectrum) noexcept -> Stochastic_Spectrum&;
		auto operator/(Stochastic_Spectrum const& spectrum) const noexcept -> Stochastic_Spectrum;
		auto operator/=(Stochastic_Spectrum const& spectrum) noexcept -> Stochastic_Spectrum&;

		auto operator=(f32 s) noexcept -> Stochastic_Spectrum&;
		auto operator+(f32 s) const noexcept -> Stochastic_Spectrum;
		auto operator+=(f32 s) noexcept -> Stochastic_Spectrum&;
		auto operator-(f32 s) const noexcept -> Stochastic_Spectrum;
		auto operator-() const noexcept -> Stochastic_Spectrum;
		auto operator-=(f32 s) noexcept -> Stochastic_Spectrum&;
		auto operator*(f32 s) const noexcept -> Stochastic_Spectrum;
		auto operator*=(f32 s) noexcept -> Stochastic_Spectrum&;
		auto operator/(f32 s) const noexcept -> Stochastic_Spectrum;
		auto operator/=(f32 s) noexcept -> Stochastic_Spectrum&;

		explicit operator bool() const noexcept;
	};

	auto operator+(f32 s, Stochastic_Spectrum const& spectrum) noexcept -> Stochastic_Spectrum;
	auto operator-(f32 s, Stochastic_Spectrum const& spectrum) noexcept -> Stochastic_Spectrum;
	auto operator*(f32 s, Stochastic_Spectrum const& spectrum) noexcept -> Stochastic_Spectrum;
	auto operator/(f32 s, Stochastic_Spectrum const& spectrum) noexcept -> Stochastic_Spectrum;

	auto min(Stochastic_Spectrum const& spectrum) noexcept -> f32;
	auto max(Stochastic_Spectrum const& spectrum) noexcept -> f32;
	auto avg(Stochastic_Spectrum const& spectrum) noexcept -> f32;

	auto constant(Stochastic_Spectrum const& spectrum) noexcept -> bool;
	auto coherent(Stochastic_Spectrum const& spectrum) noexcept -> bool;
	auto degrade(Stochastic_Spectrum& spectrum) noexcept -> void;
}
