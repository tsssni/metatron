#include <metatron/resource/opaque/grid.hpp>
#include <functional>

namespace mtt::opaque {
    auto Grid::operator[](usize x, usize y, usize z) noexcept -> f32& {
        auto offset = (z * width * height + y * width + x);
        return cells[offset];
    }

    auto Grid::operator[](usize x, usize y, usize z) const noexcept -> f32 {
        return const_cast<ref<Grid>>(*this)[x, y, z];
    }

    auto Grid::operator()(cref<fv3> uvw) const -> fv4 {
        auto pixel = uvw * fv3{size};
        auto base = math::clamp(math::floor(pixel - 0.5f), fv3{0.f}, fv3{size} - 2);
        auto frac = math::clamp(pixel - 0.5f - base, fv3{0.f}, fv3{1.f});

        auto weights = std::array<std::function<auto(f32) -> f32>, 2>{
            [](f32 x) { return 1.f - x; },
            [](f32 x) { return x; },
        };
        auto offsets = iv2{0, 1};

        auto r = fv4{0.f};
        for (auto i = 0; i < 4; ++i) {
            auto b0 = i & 1;
            auto b1 = (i & 2) >> 1;
            auto w = weights[b0](frac[0]) * weights[b1](frac[1]);
            auto o = base + uzv2{offsets[b0], offsets[b1]};

            if (base[2] + 1 >= depth)
                r += w * fv4{(*this)[o[0], o[1], base[2]]};
            else
                r += w * math::lerp(
                    fv4{(*this)[o[0], o[1], base[2]]},
                    fv4{(*this)[o[0], o[1], base[2] + 1]},
                    frac[2]
                );
        }
        return r;
    }
}
