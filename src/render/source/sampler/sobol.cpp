#include <metatron/render/sampler/sobol.hpp>
#include <metatron/core/math/low-discrepancy.hpp>
#include <metatron/core/math/encode.hpp>
#include <metatron/core/math/hash.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/print.hpp>
#include <fstream>

namespace mtt::sampler {
    inline buf<u32> Sobol_Sampler::sobol_matrices;

    // avoid extra parameters uploaded to gpu
    Sobol_Sampler::Sobol_Sampler(cref<Descriptor>) noexcept: matrices(std::span<u32>(sobol_matrices)) {}

    auto Sobol_Sampler::init() noexcept -> void {
        auto path = "sampler/sobol.bin";
        auto data = stl::filesystem::find(path);

        auto f = std::ifstream{data, std::ios::binary};
        if (!f.is_open()) stl::abort("{} not open", path);

        auto size = 0ull;
        f.read(mut<char>(&size), sizeof(size));
        sobol_matrices = size;
        f.read(mut<char>(sobol_matrices.ptr), sobol_matrices.bytelen);
        f.close();
    }

    auto Sobol_Sampler::start(ref<Context> ctx) const noexcept -> void {
        auto log2_spp = u32(math::log2i(ctx.spp));
        auto res = std::bit_ceil(u32(math::max(ctx.size)));
        auto log4_spp = (log2_spp + 1) / 2;
        auto base4_digits = math::log2i(res) + log4_spp;
        auto morton_idx = (math::morton_encode(ctx.pixel) << log2_spp) | ctx.idx;

        ctx.data[0] = log2_spp;
        ctx.data[1] = base4_digits;
        ctx.data[2] = morton_idx;
    }

    auto Sobol_Sampler::generate_1d(ref<Context> ctx) const noexcept -> f32 {
        auto idx = permute_idx(ctx);
        ++ctx.dim;
        return sobol(idx, 0, u32(math::murmur_hash(ctx.dim, ctx.seed)));
    }

    auto Sobol_Sampler::generate_2d(ref<Context> ctx) const noexcept -> fv2 {
        auto idx = permute_idx(ctx);
        ctx.dim += 2;
        auto bits = math::murmur_hash(ctx.dim, ctx.seed);
        return {
            sobol(idx, 0, bits),
            sobol(idx, 1, bits >> 32),
        };
    }

    auto Sobol_Sampler::generate_pixel_2d(ref<Context> ctx) const noexcept -> fv2 {
        return generate_2d(ctx);
    }

    auto Sobol_Sampler::permute_idx(cref<Context> ctx) const noexcept -> u64 {
        auto constexpr permutations = bm<24, 4>{
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
            {3, 0, 1, 2}
        };

        auto log2_spp = ctx.data[0];
        auto base4_digits = ctx.data[1];
        auto morton_idx = ctx.data[2];

        auto idx = 0ull;
        // apply random permutations to full base-4 digits
        auto last_digit = i32(log2_spp & 1);
        for (auto i = i32(base4_digits) - 1; i >= last_digit; --i) {
            // randomly permute ith base-4 digit in morton index
            auto shift = 2 * i - last_digit;
            auto digit = (morton_idx >> shift) & 3;
            // choose permutation p to use for digit
            auto higher_digits = morton_idx >> (shift + 2);
            auto p = (math::mix_bits(higher_digits ^ (0x55555555u * ctx.dim)) >> 24) % 24;
            idx |= u64(permutations[p][digit]) << shift;
        }

        // handle power-of-2 (but not 4) spp
        if (last_digit == 1) {
            int digit = morton_idx & 1;
            idx |= digit ^ (math::mix_bits((morton_idx >> 1) ^ (0x55555555u * ctx.dim)) & 1);
        }
        return idx;
    }

    auto Sobol_Sampler::sobol(u64 idx, i32 dim, u32 hash) const noexcept -> f32 {
        auto x = 0u;
        for (auto i = dim * sobol_matrix_size; idx != 0; idx >>= 1, ++i)
            if (idx & 1) x ^= matrices[i];

        x = math::fast_binary_owen_scramble(x, hash);
        return math::min(x * 0x1p-32f, 1.f - math::epsilon<f32>);
    }
}
