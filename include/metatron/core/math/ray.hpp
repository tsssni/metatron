#pragma once
#include <metatron/core/math/vector.hpp>

namespace mtt::math {
    struct Ray final {
        fv3 o;
        fv3 d;
    };

    struct Ray_Differential final {
        Ray r;
        Ray rx;
        Ray ry;
        bool differentiable;
    };
}
