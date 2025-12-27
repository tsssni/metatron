#pragma once
#include <metatron/resource/texture/texture.hpp>
#include <metatron/resource/bsdf/bsdf.hpp>
#include <metatron/resource/media/medium.hpp>

namespace mtt::material {
    struct Interaction final {
        obj<bsdf::Bsdf> bsdf;
        fv4 emission;
        fv3 normal = {0.f};
        bool degraded = false;
    };

    MTT_POLY_METHOD(material_sample, sample);
    MTT_POLY_METHOD(material_flags, flags);

    enum Flags {
        interface = 1 << 0,
        emissive = 1 << 1,
    };

    struct Material final: pro::facade_builder
    ::add_convention<material_sample, auto (
        cref<math::Context> ctx,
        cref<muldim::Coordinate> coord
    ) const noexcept -> opt<Interaction>>
    ::add_convention<material_flags, auto () const noexcept -> Flags>
    ::build {};
}
