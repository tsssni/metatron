#pragma once
#include <metatron/resource/phase/phase-function.hpp>
#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/resource/eval/context.hpp>
#include <metatron/core/math/ray.hpp>

namespace mtt::media {
    struct Interaction final {
        math::Vector<f32, 3> p;
        poly<phase::Phase_Function> phase;
        f32 t;
        f32 pdf;
        spectra::Stochastic_Spectrum spectra_pdf;
        spectra::Stochastic_Spectrum transmittance;
        spectra::Stochastic_Spectrum sigma_a;
        spectra::Stochastic_Spectrum sigma_s;
        spectra::Stochastic_Spectrum sigma_n;
        spectra::Stochastic_Spectrum sigma_maj;
        spectra::Stochastic_Spectrum sigma_e;
    };

    MTT_POLY_METHOD(medium_sample, sample);

    struct Medium final: pro::facade_builder
    ::add_convention<medium_sample, auto (
        eval::Context const& ctx, f32 t_max, f32 u
    ) const noexcept -> std::optional<Interaction>>
    ::add_skill<pro::skills::as_view>
    ::build {};
}
