#pragma once
#include <metatron/render/filter/filter.hpp>

namespace mtt::filter {
    struct Box_Filter final {
        struct Descriptor final {
            fv2 radius = {0.5f};
        };
        Box_Filter(cref<Descriptor> desc) noexcept;
        Box_Filter() noexcept = default;
        auto operator()(cref<fv2> p) const noexcept -> f32;
        auto sample(cref<fv2> u) const noexcept -> opt<Interaction>;

    private:
        fv2 radius;
    };
}
