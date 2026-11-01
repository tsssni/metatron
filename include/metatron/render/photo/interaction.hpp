#pragma once
#include <metatron/core/math/ray.hpp>

namespace mtt::render {

struct Lens_Interaction final {
    core::Ray r;
    f32 pdf;
};

struct Camera_Interaction final {
    core::Ray_Differential ray_differential;
    f32 pdf;
};

}
