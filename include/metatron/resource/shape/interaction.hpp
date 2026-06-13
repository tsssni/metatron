#pragma once
#include <metatron/core/math/bounding-box.hpp>
#include <metatron/core/math/eval.hpp>

namespace mtt::shape {
    struct Interaction final {
        fv3 p;
        fv3 n;
        fv3 tn;
        fv3 bn;
        fv2 uv;
        f32 t;
        f32 pdf;

        fv3 dpdu;
        fv3 dpdv;
        fv3 dndu;
        fv3 dndv;
    };
}
