#pragma once
#include <metatron/resource/spectra/spectrum.hpp>
#include <metatron/core/math/vector.hpp>

namespace mtt::spectra {
    auto constexpr stochastic_samples = 4uz;

    struct Stochastic_Spectrum final {
        fv<stochastic_samples> lambda{};
        fv<stochastic_samples> value{};

        Stochastic_Spectrum() noexcept = default;
        Stochastic_Spectrum(f32 u, f32 v = 0.f) noexcept;

        auto operator()(f32 lambda) const noexcept -> f32;
        auto operator()(view<Spectrum> spectrum) const noexcept -> f32;
        auto operator()(tag<Spectrum> spectrum) const noexcept -> f32;
        auto operator&(view<Spectrum> spectrum) const noexcept -> Stochastic_Spectrum;
        auto operator&(tag<Spectrum> spectrum) const noexcept -> Stochastic_Spectrum;

        auto operator+(cref<Stochastic_Spectrum> spectrum) const noexcept -> Stochastic_Spectrum;
        auto operator+=(cref<Stochastic_Spectrum> spectrum) noexcept -> ref<Stochastic_Spectrum>;
        auto operator-(cref<Stochastic_Spectrum> spectrum) const noexcept -> Stochastic_Spectrum;
        auto operator-=(cref<Stochastic_Spectrum> spectrum) noexcept -> ref<Stochastic_Spectrum>;
        auto operator*(cref<Stochastic_Spectrum> spectrum) const noexcept -> Stochastic_Spectrum;
        auto operator*=(cref<Stochastic_Spectrum> spectrum) noexcept -> ref<Stochastic_Spectrum>;
        auto operator/(cref<Stochastic_Spectrum> spectrum) const noexcept -> Stochastic_Spectrum;
        auto operator/=(cref<Stochastic_Spectrum> spectrum) noexcept -> ref<Stochastic_Spectrum>;

        auto operator=(f32 s) noexcept -> ref<Stochastic_Spectrum>;
        auto operator+(f32 s) const noexcept -> Stochastic_Spectrum;
        auto operator+=(f32 s) noexcept -> ref<Stochastic_Spectrum>;
        auto operator-(f32 s) const noexcept -> Stochastic_Spectrum;
        auto operator-() const noexcept -> Stochastic_Spectrum;
        auto operator-=(f32 s) noexcept -> ref<Stochastic_Spectrum>;
        auto operator*(f32 s) const noexcept -> Stochastic_Spectrum;
        auto operator*=(f32 s) noexcept -> ref<Stochastic_Spectrum>;
        auto operator/(f32 s) const noexcept -> Stochastic_Spectrum;
        auto operator/=(f32 s) noexcept -> ref<Stochastic_Spectrum>;

        explicit operator bool() const noexcept;
    };

    auto operator+(f32 s, cref<Stochastic_Spectrum> spectrum) noexcept -> Stochastic_Spectrum;
    auto operator-(f32 s, cref<Stochastic_Spectrum> spectrum) noexcept -> Stochastic_Spectrum;
    auto operator*(f32 s, cref<Stochastic_Spectrum> spectrum) noexcept -> Stochastic_Spectrum;
    auto operator/(f32 s, cref<Stochastic_Spectrum> spectrum) noexcept -> Stochastic_Spectrum;

    auto min(cref<Stochastic_Spectrum> spectrum) noexcept -> f32;
    auto min(cref<Stochastic_Spectrum> spectrum, fv4 x) noexcept -> Stochastic_Spectrum;
    auto max(cref<Stochastic_Spectrum> spectrum) noexcept -> f32;
    auto max(cref<Stochastic_Spectrum> spectrum, fv4 x) noexcept -> Stochastic_Spectrum;
    auto avg(cref<Stochastic_Spectrum> spectrum) noexcept -> f32;

    auto valid(cref<Stochastic_Spectrum> spectrum) noexcept -> bool;
    auto constant(cref<Stochastic_Spectrum> spectrum) noexcept -> bool;
    auto coherent(cref<Stochastic_Spectrum> spectrum) noexcept -> bool;
    auto degrade(ref<Stochastic_Spectrum> spectrum) noexcept -> void;
}

namespace mtt {
    using stsp = spectra::Stochastic_Spectrum;
}
