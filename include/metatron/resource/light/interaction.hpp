#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/resource/spectra/spectrum.hpp>

namespace mtt::light {
    struct Interaction final {
        fv4 L;
        fv3 wi;
        fv3 p;
        f32 t;
        f32 pdf;
    };

    enum Flags {
        delta = 1 << 0,
        inf = 1 << 1,
    };
}
