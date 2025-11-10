#pragma once
#include <metatron/core/math/bounding-box.hpp>

namespace mtt::volume {
    MTT_POLY_METHOD(volume_to_local, to_local);
    MTT_POLY_METHOD(volume_to_index, to_index);
    MTT_POLY_METHOD(volume_dimensions, dimensions);
    MTT_POLY_METHOD(volume_inside, inside);
    MTT_POLY_METHOD(volume_bounding_box, bounding_box);

    struct Volume final: pro::facade_builder
    ::add_convention<volume_to_local, auto (
        math::Vector<i32, 3> const& ijk
    ) const noexcept -> math::Vector<f32, 3>>
    ::add_convention<volume_to_index, auto (
        math::Vector<f32, 3> const& pos
    ) const noexcept -> math::Vector<i32, 3>>
    ::add_convention<volume_dimensions, auto (
    ) const noexcept -> math::Vector<usize, 3>>
    ::add_convention<volume_inside,
        auto (math::Vector<i32, 3> const& pos) const noexcept -> bool,
        auto (math::Vector<f32, 3> const& pos) const noexcept -> bool
    >
    ::add_convention<volume_bounding_box,
        auto () const noexcept -> math::Bounding_Box,
        auto (math::Vector<f32, 3> const& pos) const noexcept -> math::Bounding_Box,
        auto (math::Vector<i32, 3> const& ijk) const noexcept -> math::Bounding_Box
    >
    ::template add_convention<pro::operator_dispatch<"()">,
        auto (math::Vector<f32, 3> const& pos) const noexcept -> f32
    >
    ::template add_convention<pro::operator_dispatch<"[]">,
        auto (math::Vector<i32, 3> const& ijk) noexcept -> f32&,
        auto (math::Vector<i32, 3> const& ijk) const noexcept -> f32
    >
    ::template add_skill<pro::skills::as_view>
    ::build {};
}
