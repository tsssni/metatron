#pragma once
#include <metatron/resource/texture/texture.hpp>
#include <metatron/resource/bsdf/bsdf.hpp>

namespace mtt::material {
    struct Interaction final {
        bsdf::Bsdf bsdf;
        fv4 emission;
        fv3 normal = {0.f};
        bool degraded = false;
    };

    enum Flags {
        interface = 1 << 0,
        emissive = 1 << 1,
    };
}
