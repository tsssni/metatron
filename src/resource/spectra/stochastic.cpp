#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/math/distribution/spectrum.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::spectra {
    Stochastic_Spectrum::Stochastic_Spectrum(f32 u, f32 v) noexcept {
        lambda = math::foreach([&](f32 l, usize i) {
            auto ui = math::mod(u + i / f32(stochastic_samples), 1.f);
            return math::Spectrum_Distribution{}.sample(ui);
        }, lambda);
        value = fv<stochastic_samples>{v};
    }

    auto Stochastic_Spectrum::operator()(f32 lambda) const noexcept -> f32 {
        for (auto i = 0uz; i < stochastic_samples; ++i)
            if (lambda == this->lambda[i]) return value[i];
        std::println("spectra: no matched lambda in stochastic spectrum");
        std::abort();
    }

    auto Stochastic_Spectrum::operator()(view<Spectrum> spectrum) const noexcept -> f32 {
        auto pdf = math::foreach([&](f32 l, usize i) {
            return math::Spectrum_Distribution{}.pdf(l);
        }, lambda);
        auto spec = (*this * (*this & spectrum)).value / pdf;
        return math::sum(spec) / stochastic_samples;
    }

    auto Stochastic_Spectrum::operator()(tag<Spectrum> spectrum) const noexcept -> f32 {
        return (*this)(spectrum.data());
    }

    auto Stochastic_Spectrum::operator&(view<Spectrum> spectrum) const noexcept -> Stochastic_Spectrum {
        auto spec = *this;
        spec.value = math::foreach([&](f32 lambda, usize i) {
            return (*spectrum)(lambda);
        }, lambda);
        return spec;
    }

    auto Stochastic_Spectrum::operator&(tag<Spectrum> spectrum) const noexcept -> Stochastic_Spectrum {
        return (*this) & spectrum.data();
    }

    auto Stochastic_Spectrum::operator+(cref<Stochastic_Spectrum> spectrum) const noexcept -> Stochastic_Spectrum {
        auto spec = *this;
        spec += spectrum;
        return spec;
    }
    
    auto Stochastic_Spectrum::operator+=(cref<Stochastic_Spectrum> spectrum) noexcept -> ref<Stochastic_Spectrum> {
        value += math::foreach([&](f32 lambda, usize i) {
            return spectrum(lambda);
        }, lambda);
        return *this;
    }

    auto Stochastic_Spectrum::operator-(cref<Stochastic_Spectrum> spectrum) const noexcept -> Stochastic_Spectrum {
        auto spec = *this;
        spec -= spectrum;
        return spec;
    }
    
    auto Stochastic_Spectrum::operator-=(cref<Stochastic_Spectrum> spectrum) noexcept -> ref<Stochastic_Spectrum> {
        value -= math::foreach([&](f32 lambda, usize i) {
            return spectrum(lambda);
        }, lambda);
        return *this;
    }

    auto Stochastic_Spectrum::operator*(cref<Stochastic_Spectrum> spectrum) const noexcept -> Stochastic_Spectrum {
        auto spec = *this;
        spec *= spectrum;
        return spec;
    }
    
    auto Stochastic_Spectrum::operator*=(cref<Stochastic_Spectrum> spectrum) noexcept -> ref<Stochastic_Spectrum> {
        value *= math::foreach([&](f32 lambda, usize i) {
            return spectrum(lambda);
        }, lambda);
        return *this;
    }

    auto Stochastic_Spectrum::operator/(cref<Stochastic_Spectrum> spectrum) const noexcept -> Stochastic_Spectrum {
        auto spec = *this;
        spec /= spectrum;
        return spec;
    }
    
    auto Stochastic_Spectrum::operator/=(cref<Stochastic_Spectrum> spectrum) noexcept -> ref<Stochastic_Spectrum> {
        value = math::foreach([&](f32 value, f32 lambda, usize i) {
            return math::guarded_div(value, spectrum(lambda));
        }, value, lambda);
        return *this;
    }

    auto Stochastic_Spectrum::operator=(f32 s) noexcept -> ref<Stochastic_Spectrum> {
        value = fv<stochastic_samples>{s};
        return *this;
    }

    auto Stochastic_Spectrum::operator+(f32 s) const noexcept -> Stochastic_Spectrum {
        auto spec = *this;
        spec += s;
        return spec;
    };

    auto Stochastic_Spectrum::operator+=(f32 s) noexcept -> ref<Stochastic_Spectrum> {
        value += fv<stochastic_samples>{s};
        return *this;
    };

    auto Stochastic_Spectrum::operator-(f32 s) const noexcept -> Stochastic_Spectrum {
        auto spec = *this;
        spec -= s;
        return spec;
    };

    auto Stochastic_Spectrum::operator-() const noexcept -> Stochastic_Spectrum {
        auto spec = *this;
        spec.value = -spec.value;
        return spec;
    };

    auto Stochastic_Spectrum::operator-=(f32 s) noexcept -> ref<Stochastic_Spectrum> {
        value -= fv<stochastic_samples>{s};
        return *this;
    };

    auto Stochastic_Spectrum::operator*(f32 s) const noexcept -> Stochastic_Spectrum {
        auto spec = *this;
        spec *= s;
        return spec;
    };

    auto Stochastic_Spectrum::operator*=(f32 s) noexcept -> ref<Stochastic_Spectrum> {
        value *= fv<stochastic_samples>{s};
        return *this;
    };

    auto Stochastic_Spectrum::operator/(f32 s) const noexcept -> Stochastic_Spectrum {
        auto spec = *this;
        spec /= s;
        return spec;
    };

    auto Stochastic_Spectrum::operator/=(f32 s) noexcept -> ref<Stochastic_Spectrum> {
        value /= fv<stochastic_samples>{s};
        return *this;
    };

    Stochastic_Spectrum::operator bool() const noexcept {
        return math::any([](f32 x, usize i) { return x > 0.f; }, value);
    }

    auto operator+(f32 s, cref<Stochastic_Spectrum> spectrum) noexcept -> Stochastic_Spectrum {
        return spectrum + s;
    }

    auto operator-(f32 s, cref<Stochastic_Spectrum> spectrum) noexcept -> Stochastic_Spectrum {
        return -spectrum + s;
    }

    auto operator*(f32 s, cref<Stochastic_Spectrum> spectrum) noexcept -> Stochastic_Spectrum {
        return spectrum * s;
    }

    auto operator/(f32 s, cref<Stochastic_Spectrum> spectrum) noexcept -> Stochastic_Spectrum {
        auto spec = spectrum;
        spec.value = math::foreach([&](f32 v, usize i) {
            return math::guarded_div(s, v);
        }, spectrum.value);
        return spec;
    }

    auto min(cref<Stochastic_Spectrum> spectrum) noexcept -> f32 {
        return math::min(spectrum.value);
    }

    auto min(cref<Stochastic_Spectrum> spectrum, fv4 x) noexcept -> Stochastic_Spectrum {
        auto spec = spectrum;
        spec.value = math::min(spec.value, x);
        return spec;
    }

    auto max(cref<Stochastic_Spectrum> spectrum) noexcept -> f32 {
        return math::max(spectrum.value);
    }

    auto max(cref<Stochastic_Spectrum> spectrum, fv4 x) noexcept -> Stochastic_Spectrum {
        auto spec = spectrum;
        spec.value = math::max(spec.value, x);
        return spec;
    }

    auto avg(cref<Stochastic_Spectrum> spectrum) noexcept -> f32 {
        return math::sum(spectrum.value / stochastic_samples);
    }

    auto valid(cref<Stochastic_Spectrum> spectrum) noexcept -> bool {
        return spectrum.lambda[0] != 0.f;
    }

    auto constant(cref<Stochastic_Spectrum> spectrum) noexcept -> bool {
        return math::all([&](f32 x, usize i) { return x == spectrum.value[0]; }, spectrum.value);
    }

    auto coherent(cref<Stochastic_Spectrum> spectrum) noexcept -> bool {
        return math::all([&](f32 x, usize i) { return x == spectrum.lambda[0]; }, spectrum.lambda);
    }

    auto degrade(ref<Stochastic_Spectrum> spectrum) noexcept -> void {
        spectrum.value = {spectrum.value[0]};
        spectrum.lambda = {spectrum.lambda[0]};
    }
}
