#pragma once
#include <metatron/render/spectra/spectrum.hpp>

namespace mtt::render {

struct Light_Interaction final {
    fv4 L;
    fv3 wi;
    fv3 p;
    f32 t;
    f32 pdf;
};

enum Light_Flags {
    delta = 1 << 0,
    inf = 1 << 1,
};

}
