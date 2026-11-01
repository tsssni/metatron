#include <metatron/render/sampler/z-sobol.hpp>
#include <metatron/core/math/encode.hpp>
#include <metatron/core/math/low-discrepancy.hpp>
#include <metatron/core/stl/filesystem.hpp>

namespace mtt::render {

auto constexpr num_sobol_dimensions = 2u;
auto constexpr sobol_matrix_size = 52u;

Z_Sobol_Sampler::Z_Sobol_Sampler() noexcept {
    auto file = std::ifstream{core::filesystem::find("sampler/sobol.bin"), std::ios::binary};
    auto size = 0ull;
    file.read((char*)&size, sizeof(size));
    matrices = size;
    matrices.map();
    file.read((char*)matrices.ptr, size * sizeof(u32));
    matrices.unmap();
    file.close();
}

auto Z_Sobol_Sampler::start(Sampler_Context& ctx) const noexcept -> void {
    auto log2_spp = u32(core::log2i(ctx.spp));
    auto res = std::bit_ceil(u32(core::max(ctx.size)));
    auto log4_spp = (log2_spp + 1) / 2;
    auto base4_digits = core::log2i(res) + log4_spp;
    auto morton_idx = (core::morton_encode(ctx.pixel) << log2_spp) | ctx.idx;

    ctx.data[0] = log2_spp;
    ctx.data[1] = base4_digits;
    ctx.data[2] = morton_idx;
}

auto Z_Sobol_Sampler::generate_1d(Sampler_Context& ctx) const noexcept -> f32 {
    auto idx = permute_idx(ctx);
    ++ctx.dim;
    return sobol(idx, 0, u32(core::murmur_hash(ctx.dim, ctx.seed)));
}

auto Z_Sobol_Sampler::generate_2d(Sampler_Context& ctx) const noexcept -> fv2 {
    auto idx = permute_idx(ctx);
    ctx.dim += 2;
    auto bits = core::murmur_hash(ctx.dim, ctx.seed);
    return {
        sobol(idx, 0, u32(bits)),
        sobol(idx, 1, u32(bits >> 32)),
    };
}

auto Z_Sobol_Sampler::generate_pixel_2d(Sampler_Context& ctx) const noexcept -> fv2 {
    return generate_2d(ctx);
}

auto Z_Sobol_Sampler::permute_idx(Sampler_Context const& ctx) const noexcept -> u64 {
    auto constexpr permutations = [] {
        // precompute 256 permutations to avoid mod 24
        auto p = std::array<std::array<u8, 4>, 256>{{
            {0, 1, 2, 3},
            {0, 1, 3, 2},
            {0, 2, 1, 3},
            {0, 2, 3, 1},
            {0, 3, 2, 1},
            {0, 3, 1, 2},
            {1, 0, 2, 3},
            {1, 0, 3, 2},
            {1, 2, 0, 3},
            {1, 2, 3, 0},
            {1, 3, 2, 0},
            {1, 3, 0, 2},
            {2, 1, 0, 3},
            {2, 1, 3, 0},
            {2, 0, 1, 3},
            {2, 0, 3, 1},
            {2, 3, 0, 1},
            {2, 3, 1, 0},
            {3, 1, 2, 0},
            {3, 1, 0, 2},
            {3, 2, 1, 0},
            {3, 2, 0, 1},
            {3, 0, 2, 1},
            {3, 0, 1, 2},
        }};
        for (auto i = 24; i < 256; i++)
            p[i] = p[i % 24];
        return p;
    }();

    auto log2_spp = ctx.data[0];
    auto base4_digits = ctx.data[1];
    auto morton_idx = ctx.data[2];

    // apply random permutations to full base-4 digits
    auto idx = 0ull;
    auto last_digit = i32(log2_spp & 1);
    for (auto i = i32(base4_digits) - 1; i >= last_digit; --i) {
        auto shift = 2 * i - last_digit;
        auto digit = (morton_idx >> shift) & 3;
        auto higher_digits = morton_idx >> (shift + 2);
        auto p = core::mix_bits(higher_digits ^ (0x55555555u * ctx.dim)) >> 24;
        idx |= u64(permutations[p][digit]) << shift;
    }

    // handle power-of-2 (but not 4) spp
    if (last_digit == 1) {
        auto digit = morton_idx & 1;
        idx |= digit ^ (core::mix_bits((morton_idx >> 1) ^ (0x55555555u * ctx.dim)) & 1);
    }
    return idx;
}

auto Z_Sobol_Sampler::sobol(u64 idx, i32 dim, u32 hash) const noexcept -> f32 {
    auto x = 0u;
    auto offset = dim * sobol_matrix_size;
    while (idx != 0) {
        x ^= matrices[offset + std::countr_zero(idx)];
        idx &= idx - 1;
    }

    x = core::fast_binary_owen_scramble(x, hash);
    return core::min(x * 0x1p-32f, 1.f - core::epsilon<f32>);
}

}
