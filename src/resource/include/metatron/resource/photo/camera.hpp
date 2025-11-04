#pragma once
#include <metatron/resource/photo/film.hpp>
#include <metatron/resource/photo/lens/lens.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/math/sampler/sampler.hpp>

namespace mtt::photo {
    struct Interaction final {
        math::Ray_Differential ray_differential;
        math::Ray_Differential default_differential;
        Fixel fixel;
    };

    struct Camera final {
        stl::proxy<Lens> const lens;
        stl::proxy<Film> film;

        auto sample(
            math::Vector<usize, 2> pixel,
            usize idx,
            mut<math::Sampler> sampler
        ) noexcept -> std::optional<Interaction>;
    };
}
