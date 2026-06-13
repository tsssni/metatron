#include <metatron/render/sampler/independent.hpp>
#include <metatron/core/math/hash.hpp>

namespace mtt::sampler {
    auto Independent_Sampler::start(ref<Context> ctx) const noexcept -> void {
        auto seed = u32(math::murmur_hash(ctx.pixel, ctx.idx, ctx.seed));
        auto rng = std::minstd_rand{seed};
        auto distr = std::uniform_real_distribution<f32>{1e-4f, 1.f - 1e-4f};
        std::memcpy(&ctx.data[0], &rng, sizeof(rng));
        std::memcpy(&ctx.data[2], &distr, sizeof(distr));
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
