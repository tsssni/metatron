#pragma once
#include <metatron/render/photo/film.hpp>
#include <metatron/render/photo/lens/lens.hpp>
#include <metatron/render/sampler/sampler.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/ray.hpp>

namespace mtt::photo {
    struct Interaction final {
        math::Ray_Differential ray_differential;
        math::Ray_Differential default_differential;
        f32 pdf;
    };

    struct Camera final {
        auto sample(
            view<Lens> lens,
            math::Vector<f32, 2> pos,
            math::Vector<f32, 2> dxdy,
            math::Vector<f32, 2> u
        ) noexcept -> std::optional<Interaction>;
    };
}
