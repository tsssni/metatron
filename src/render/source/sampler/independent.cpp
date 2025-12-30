#include <metatron/render/sampler/independent.hpp>
#include <cstring>

namespace mtt::sampler {
    auto Independent_Sampler::start(ref<Context> ctx) const noexcept -> void {
        auto rng = std::minstd_rand{u32(ctx.seed)};
        auto distr = std::uniform_real_distribution<f32>{1e-4f, 1.f - 1e-4f};
        std::memcpy(&ctx.data[0], &rng, sizeof(rng));
        std::memcpy(&ctx.data[2], &rng, sizeof(distr));
    }

    auto Independent_Sampler::generate_1d(ref<Context> ctx) const noexcept -> f32 {
        auto& rng = *mut<std::minstd_rand>(&ctx.data[0]);
        auto& distr = *mut<std::uniform_real_distribution<f32>>(&ctx.data[2]);
        return distr(rng);
    }

    auto Independent_Sampler::generate_2d(ref<Context> ctx) const noexcept -> fv2 {
        return {generate_1d(ctx), generate_1d(ctx)};
    }

    auto Independent_Sampler::generate_pixel_2d(ref<Context> ctx) const noexcept -> fv2 {
        return generate_2d(ctx);
    }
}
