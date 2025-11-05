#pragma once
#include <metatron/render/filter/filter.hpp>

namespace mtt::filter {
    struct Box_Filter final {
        Box_Filter(math::Vector<f32, 2> const& radius = {0.5f}) noexcept;
        auto operator()(math::Vector<f32, 2> const& p) const noexcept -> f32;
        auto sample(math::Vector<f32, 2> const& u) const noexcept -> std::optional<filter::Interaction>;

    private:
        math::Vector<f32, 2> radius;
    };
}
