#include <metatron/render/sampler/heitz.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/hash.hpp>
#include <metatron/core/stl/print.hpp>
#include <heitz/heitz.h>
#include <cstring>

namespace mtt::sampler {
    buf<u32> Heitz_Sampler::heitz_sobol;
    buf<u32> Heitz_Sampler::heitz_ranking;
    buf<u32> Heitz_Sampler::heitz_scrambling;

    Heitz_Sampler::Heitz_Sampler(cref<Descriptor>) noexcept:
    sobol(std::span<u32>(heitz_sobol)),
    ranking(std::span<u32>(heitz_ranking)),
    scrambling(std::span<u32>(heitz_scrambling)) {}

    auto Heitz_Sampler::init() noexcept -> void {
        heitz_sobol = heitz_sobol_n * heitz_sobol_d;
        auto* sd = mut<u32>(heitz_sobol.ptr);
        for (auto i = 0u; i < heitz_sobol_n * heitz_sobol_d; ++i)
            sd[i] = u32(sobol_256spp_256d[i]);

        auto tile_count = heitz_tile_size * heitz_tile_size * heitz_tile_d;
        heitz_ranking = tile_count;
        auto* rd = mut<u32>(heitz_ranking.ptr);
        for (auto i = 0u; i < tile_count; ++i)
            rd[i] = u32(rankingTile[i]);

        heitz_scrambling = tile_count;
        auto* cd = mut<u32>(heitz_scrambling.ptr);
        for (auto i = 0u; i < tile_count; ++i)
            cd[i] = u32(scramblingTile[i]);
    }

    auto Heitz_Sampler::start(ref<Context> ctx) const noexcept -> void {}

    auto sample(
        cref<buf<u32>> sobol,
        cref<buf<u32>> ranking,
        cref<buf<u32>> scrambling,
        u32 px, u32 py, u32 sample_idx, u32 dim, u32 seed
    ) noexcept -> f32 {
        auto i = px & (heitz_tile_size - 1);
        auto j = py & (heitz_tile_size - 1);
        auto batch = sample_idx / heitz_spp;
        auto s = sample_idx & (heitz_spp - 1);
        auto d_full = dim & (heitz_sobol_d - 1);
        auto d_tile = dim & (heitz_tile_d - 1);

        auto tile_pixel = (i + j * heitz_tile_size) * heitz_tile_d;
        auto ranked = s ^ ranking[d_tile + tile_pixel];
        auto value = sobol[d_full + ranked * heitz_sobol_d];
        value = value ^ scrambling[d_tile + tile_pixel];
        value = (value ^ math::mix_bits(batch ^ seed)) & 0xffu;
        return (0.5f + f32(value)) / 256.0f;
    }

    auto Heitz_Sampler::generate_1d(ref<Context> ctx) const noexcept -> f32 {
        auto v = sample(sobol, ranking, scrambling, ctx.pixel[0], ctx.pixel[1], ctx.idx, ctx.dim, ctx.seed);
        ++ctx.dim;
        return v;
    }

    auto Heitz_Sampler::generate_2d(ref<Context> ctx) const noexcept -> fv2 {
        auto ux = sample(sobol, ranking, scrambling, ctx.pixel[0], ctx.pixel[1], ctx.idx, ctx.dim, ctx.seed);
        auto uy = sample(sobol, ranking, scrambling, ctx.pixel[0], ctx.pixel[1], ctx.idx, ctx.dim + 1, ctx.seed);
        ctx.dim += 2;
        return {ux, uy};
    }

    auto Heitz_Sampler::generate_pixel_2d(ref<Context> ctx) const noexcept -> fv2 {
        return generate_2d(ctx);
    }
}
