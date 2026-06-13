#pragma once
#include <metatron/resource/media/interaction.hpp>

namespace mtt::media {
    struct Vaccum_Medium final {
        struct Descriptor final {};
        Vaccum_Medium(cref<Descriptor>) noexcept;
        Vaccum_Medium() noexcept = default;

        struct Iterator final {
            math::Ray r;
            f32 t_max;
            auto march(f32 u) noexcept -> opt<Interaction>;
        };

        auto begin(
            cref<math::Context> ctx, f32 t_max
        ) const noexcept -> Iterator;

    private:
        u32 padding;
    };
}
