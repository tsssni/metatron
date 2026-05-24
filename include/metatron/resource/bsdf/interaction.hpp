#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/eval.hpp>

namespace mtt::bsdf {
    struct Interaction final {
        fv4 f;
        fv3 wi;
        f32 pdf;
        bool degraded = false;
    };

    enum Flags {
        reflective = 1 << 0,
        transmissive = 1 << 1,
        specular = 1 << 2,
        interface = 1 << 3,
    };
}
