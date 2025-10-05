#pragma once
#include <metatron/resource/eval/context.hpp>
#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/core/math/ray.hpp>

namespace mtt::light {
    struct Interaction final {
        spectra::Stochastic_Spectrum L;
        math::Vector<f32, 3> wi;
        math::Vector<f32, 3> p;
        f32 t;
    };

    enum Flags {
        delta = 1 << 0,
        inf = 1 << 1,
    };

    MTT_POLY_METHOD(light_sample, sample);
    MTT_POLY_METHOD(light_pdf, pdf);
    MTT_POLY_METHOD(light_flags, flags);

    struct Light final: pro::facade_builder
    ::add_convention<pro::operator_dispatch<"()">, auto (
        math::Ray const& r,
        spectra::Stochastic_Spectrum const& spec
    ) const noexcept -> std::optional<Interaction>>
    ::add_convention<light_sample, auto (
        eval::Context const& ctx,
        math::Vector<f32, 2> const& u
    ) const noexcept -> std::optional<Interaction>>
    ::add_convention<light_pdf, auto (
        math::Ray const& r,
        math::Vector<f32, 3> const& np
    ) const noexcept -> f32>
    ::add_convention<light_flags, auto () const noexcept -> Flags>
    ::add_skill<pro::skills::as_view>
    ::build {};
}
