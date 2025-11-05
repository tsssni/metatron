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
        Fixel fixel;
    };

    struct Camera final {
        view<Lens> lens;
        mut<Film> film;

        view<filter::Filter> filter;
        mut<sampler::Sampler> sampler;

        auto sample(
            math::Vector<usize, 2> pixel,
            usize idx
        ) noexcept -> std::optional<Interaction>;
    };
}
