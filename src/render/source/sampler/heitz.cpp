#include <metatron/render/sampler/heitz.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/hash.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/print.hpp>
#include <heitz/sobol_4096spp_256d.h>
#include <cstring>
#include <fstream>

namespace mtt::sampler {
    inline buf<u32> Heitz_Sampler::heitz_sobol;
    inline buf<u32> Heitz_Sampler::heitz_mask;

    Heitz_Sampler::Heitz_Sampler(cref<Descriptor>) noexcept:
    sobol(std::span<u32>(heitz_sobol)),
    mask(std::span<u32>(heitz_mask)) {}

    auto Heitz_Sampler::init() noexcept -> void {
        heitz_sobol = heitz_sobol_n * heitz_sobol_d;
        std::memcpy(heitz_sobol.ptr, &sobol_sequence[0][0], heitz_sobol.bytelen);

        auto path = "sampler/heitz-mask.bin";
        auto data = stl::filesystem::find(path);
        auto f = std::ifstream{data, std::ios::binary};
        if (!f.is_open()) stl::abort("{} not open", path);
        auto size = 0ull;
        f.read(mut<char>(&size), sizeof(size));
        if (size != heitz_mask_size * heitz_mask_size * heitz_pair_d)
            stl::abort("{} size mismatch: {}", path, size);
        heitz_mask = size;
        f.read(mut<char>(heitz_mask.ptr), heitz_mask.bytelen);
    }

    auto Heitz_Sampler::start(ref<Context> ctx) const noexcept -> void {}

    auto Heitz_Sampler::generate_1d(ref<Context> ctx) const noexcept -> f32 {
        auto i = ctx.pixel[0] & (heitz_mask_size - 1);
        auto j = ctx.pixel[1] & (heitz_mask_size - 1);
        auto k_round = ctx.idx / heitz_sobol_n;
        auto k = ctx.idx & (heitz_sobol_n - 1);
        auto d_round = ctx.dim / heitz_sobol_d;
        auto d = ctx.dim % heitz_sobol_d;
        ++ctx.dim;

        auto extra = u32(math::murmur_hash(k_round, d_round, ctx.seed));
        auto s = sobol[k * heitz_sobol_d + d];
        auto m = mask[(i * heitz_mask_size + j) * heitz_pair_d + (d & 1)];
        return math::min((s ^ m ^ extra) * 0x1p-32f, 1.f - math::epsilon<f32>);
    }

    auto Heitz_Sampler::generate_2d(ref<Context> ctx) const noexcept -> fv2 {
        auto i = ctx.pixel[0] & (heitz_mask_size - 1);
        auto j = ctx.pixel[1] & (heitz_mask_size - 1);
        auto k_round = ctx.idx / heitz_sobol_n;
        auto k = ctx.idx & (heitz_sobol_n - 1);
        auto d_round = ctx.dim / heitz_sobol_d;
        auto d0 = ctx.dim % heitz_sobol_d;
        auto d1 = (ctx.dim + 1) % heitz_sobol_d;
        ctx.dim += 2;

        auto extra = math::murmur_hash(k_round, d_round, ctx.seed);
        auto base_mask = (i * heitz_mask_size + j) * heitz_pair_d;
        auto sx = sobol[k * heitz_sobol_d + d0] ^ mask[base_mask + (d0 & 1)] ^ u32(extra);
        auto sy = sobol[k * heitz_sobol_d + d1] ^ mask[base_mask + (d1 & 1)] ^ u32(extra >> 32);
        return {
            math::min(sx * 0x1p-32f, 1.f - math::epsilon<f32>),
            math::min(sy * 0x1p-32f, 1.f - math::epsilon<f32>),
        };
    }

    auto Heitz_Sampler::generate_pixel_2d(ref<Context> ctx) const noexcept -> fv2 {
        return generate_2d(ctx);
    }
}
