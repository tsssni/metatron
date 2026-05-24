#pragma once
#include <metatron/core/math/vector.hpp>

namespace mtt::filter {
    struct Interaction final {
        fv2 p;
        f32 weight;
        f32 pdf;
    };
}
