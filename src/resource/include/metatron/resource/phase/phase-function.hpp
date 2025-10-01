#pragma once
#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/resource/eval/context.hpp>
#include <metatron/core/math/vector.hpp>

namespace mtt::phase {
    struct Attribute final {
        spectra::Stochastic_Spectrum spectrum;
    };

    struct Interaction final {
        spectra::Stochastic_Spectrum f;
        math::Vector<f32, 3> wi;
        f32 pdf;
    };

    MTT_POLY_METHOD(phase_function_sample, sample);
    MTT_POLY_METHOD(phase_function_configure, configure);

    struct Phase_Function final: pro::facade_builder
    ::add_convention<pro::operator_dispatch<"()">, auto (
        math::Vector<f32, 3> const& wo,
        math::Vector<f32, 3> const& wi
    ) const noexcept -> std::optional<Interaction>>
    ::add_convention<phase_function_sample, auto (
        eval::Context const& ctx, math::Vector<f32, 2> const& u
    ) const noexcept -> std::optional<Interaction>>
    ::add_convention<phase_function_configure, auto (
        Attribute const& attr
    ) noexcept -> void>
    ::support_copy<pro::constraint_level::nontrivial>
    ::build {};
}
