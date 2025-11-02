#pragma once
#include <metatron/resource/texture/texture.hpp>
#include <metatron/resource/bsdf/bsdf.hpp>
#include <metatron/resource/media/medium.hpp>

namespace mtt::material {
    struct Interaction final {
        poly<bsdf::Bsdf> bsdf;
        spectra::Stochastic_Spectrum emission;
        math::Vector<f32, 3> normal{0.f};
        bool degraded{false};
    };

    MTT_POLY_METHOD(material_sample, sample);
    MTT_POLY_METHOD(material_flags, flags);

    enum Flags {
        interface,
    };

    struct Material final: pro::facade_builder
    ::add_convention<material_sample, auto (
        eval::Context const& ctx,
        device::Coordinate const& coord
    ) const noexcept -> std::optional<Interaction>>
    ::add_convention<material_flags, auto () const noexcept -> Flags>
    ::build {};
}
