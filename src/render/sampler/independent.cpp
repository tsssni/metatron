#include <metatron/render/sampler/independent.hpp>

namespace mtt::sampler {
    Independent_Sampler::Independent_Sampler() noexcept: distr(1e-4, 1.f - 1e-4) {}

    auto Independent_Sampler::start(
        math::Vector<usize, 2> const& pixel,
        usize idx, usize dim, usize seed
    ) noexcept -> void {
        this->pixel = pixel;
        this->idx = idx;
        this->dim = dim;
        rng = std::mt19937{u32(seed)};
    }

    auto Independent_Sampler::generate_1d() noexcept -> f32 {
        return distr(rng);
    }

    auto Independent_Sampler::generate_2d() noexcept -> math::Vector<f32, 2> {
        return {generate_1d(), generate_1d()};
    }

    auto Independent_Sampler::generate_pixel_2d() noexcept -> math::Vector<f32, 2> {
        return generate_2d();
    }
}
