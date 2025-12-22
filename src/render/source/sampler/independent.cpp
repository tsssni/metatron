#include <metatron/render/sampler/independent.hpp>

namespace mtt::sampler {
    Independent_Sampler::Independent_Sampler() noexcept: distr(1e-4, 1.f - 1e-4) {}

    auto Independent_Sampler::start(Context ctx) noexcept -> void {
        pixel = ctx.pixel;
        idx = ctx.idx;
        dim = ctx.dim;
        rng = std::minstd_rand{u32(ctx.seed)};
    }

    auto Independent_Sampler::generate_1d() noexcept -> f32 {
        return distr(rng);
    }

    auto Independent_Sampler::generate_2d() noexcept -> fv2 {
        return {generate_1d(), generate_1d()};
    }

    auto Independent_Sampler::generate_pixel_2d() noexcept -> fv2 {
        return generate_2d();
    }
}
