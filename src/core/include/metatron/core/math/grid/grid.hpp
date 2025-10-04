#pragma once
#include <metatron/core/math/bounding-box.hpp>

namespace mtt::math {
    MTT_POLY_METHOD(grid_to_local, to_local);
    MTT_POLY_METHOD(grid_to_index, to_index);
    MTT_POLY_METHOD(grid_bounding_box, bounding_box);

    // use i32 to avoid pos out of boundary
    template<typename T, usize x, usize y, usize z>
    struct Grid final: pro::facade_builder
    ::add_convention<grid_to_local, auto (
        math::Vector<i32, 3> const& ijk
    ) const noexcept -> math::Vector<f32, 3>>
    ::add_convention<grid_to_index, auto (
        math::Vector<f32, 3> const& pos
    ) const noexcept -> math::Vector<i32, 3>>
    ::add_convention<grid_bounding_box,
        auto () const noexcept -> math::Bounding_Box,
        auto (math::Vector<f32, 3> const& pos) const noexcept -> math::Bounding_Box,
        auto (math::Vector<i32, 3> const& ijk) const noexcept -> math::Bounding_Box
    >
    ::template add_convention<pro::operator_dispatch<"()">,
        auto (math::Vector<f32, 3> const& pos) const noexcept -> T
    >
    ::template add_convention<pro::operator_dispatch<"[]">,
        auto (math::Vector<i32, 3> const& ijk) noexcept -> T&,
        auto (math::Vector<i32, 3> const& ijk) const noexcept -> T const&
    >
    ::template add_skill<pro::skills::as_view>
    ::build {};
}
