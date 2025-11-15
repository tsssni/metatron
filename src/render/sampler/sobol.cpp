#include <metatron/render/sampler/sobol.hpp>
#include <metatron/core/math/low-discrepancy.hpp>
#include <metatron/core/math/encode.hpp>
#include <metatron/core/math/hash.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/print.hpp>
#include <fstream>

namespace mtt::sampler {
    inline std::vector<u32> Sobol_Sampler::sobol_matrices;

    auto Sobol_Sampler::init() noexcept -> void {
        auto& fs = stl::filesystem::instance();
        auto path = "sampler/sobol.bin";
        MTT_OPT_OR_CALLBACK(data, fs.find(path), {
            std::println("{} not found", path);
            std::abort();
        });

        auto f = std::ifstream{data, std::ios::binary};
        if (!f.is_open()) {
            std::println("{} not open", path);
            std::abort();
        }

        auto size = 0uz;
        f.read(mut<char>(&size), sizeof(size));
        sobol_matrices.resize(size);
        f.read(mut<char>(sobol_matrices.data()), size * sizeof(u32));
        f.close();
    }

    auto Sobol_Sampler::start(Context ctx) noexcept -> void {
        log2_spp = math::log2i(ctx.spp);
        auto res = std::bit_ceil(u32(math::max(ctx.size)));
        auto log4_spp = (log2_spp + 1) / 2;
        base4_digits = math::log2i(res) + log4_spp;

        dim = ctx.dim;
        seed = ctx.seed;
        morton_idx = (morton_encode(uv2{ctx.pixel}) << log2_spp) | ctx.idx;
    }

    auto Sobol_Sampler::generate_1d() noexcept -> f32 {
        auto idx = permute_idx();
        ++dim;
        return sobol(idx, 0, math::hash(dim, seed));
    }

    auto Sobol_Sampler::generate_2d() noexcept -> fv2 {
        auto idx = permute_idx();
        this->dim += 2;
        auto bits = math::hash(dim, seed);
        return {
            sobol(idx, 0, bits),
            sobol(idx, 1, bits >> 32),
        };
    }

    auto Sobol_Sampler::generate_pixel_2d() noexcept -> fv2 {
        return generate_2d();
    }

    auto Sobol_Sampler::permute_idx() noexcept -> usize {
        auto static constexpr permutations = bm<24, 4>{
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

        auto idx = 0uz;
        // apply random permutations to full base-4 digits
        auto last_digit = log2_spp & 1;
        for (auto i = base4_digits - 1; i >= last_digit; --i) {
            // randomly permute ith base-4 digit in morton index
            auto shift = 2 * i - last_digit;
            auto digit = (morton_idx >> shift) & 3;
            // choose permutation p to use for digit
            auto higher_digits = morton_idx >> (shift + 2);
            int p = (math::mix_bits(higher_digits ^ (0x55555555u * dim)) >> 24) % 24;

            digit = permutations[p][digit];
            idx |= u64(digit) << shift;
        }

        // handle power-of-2 (but not 4) spp
        if (last_digit == 1) {
            int digit = morton_idx & 1;
            idx |= digit ^ (math::mix_bits((morton_idx >> 1) ^ (0x55555555u * dim)) & 1);
        }
        return idx;
    }

    auto Sobol_Sampler::sobol(usize idx, i32 dim, u32 hash) noexcept -> f32 {
        auto x = 0u;
        for (auto i = dim * sobol_matrix_size; idx != 0; idx >>= 1, ++i)
            if (idx & 1) x ^= sobol_matrices[i];

        x = math::fast_binary_owen_scramble(x, hash);
        return math::min(x * 0x1p-32f, 1.f - math::epsilon<f32>);
    }
}
