#pragma once
#include <metatron/render/filter/filter.hpp>

namespace mtt::filter {
    struct Box_Filter final {
        struct Descriptor final {
            math::Vector<f32, 2> radius = {0.5f};
        };
        Box_Filter() noexcept = default;
        Box_Filter(Descriptor const& desc) noexcept;
        auto operator()(math::Vector<f32, 2> const& p) const noexcept -> f32;
        auto sample(math::Vector<f32, 2> const& u) const noexcept -> std::optional<filter::Interaction>;

    private:
        math::Vector<f32, 2> radius;
    };
}
