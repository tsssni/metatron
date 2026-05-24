#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/ray.hpp>

namespace mtt::photo::lens {
    struct Interaction final {
        math::Ray r;
        f32 pdf;
    };
}
