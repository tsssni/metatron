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
        cref<iv3> ijk
    ) const noexcept -> fv3>
    ::add_convention<volume_to_index, auto (
        cref<fv3> pos
    ) const noexcept -> iv3>
    ::add_convention<volume_dimensions, auto (
    ) const noexcept -> uzv3>
    ::add_convention<volume_inside,
        auto (cref<iv3> pos) const noexcept -> bool,
        auto (cref<fv3> pos) const noexcept -> bool
    >
    ::add_convention<volume_bounding_box,
        auto () const noexcept -> math::Bounding_Box,
        auto (cref<fv3> pos) const noexcept -> math::Bounding_Box,
        auto (cref<iv3> ijk) const noexcept -> math::Bounding_Box
    >
    ::template add_convention<pro::operator_dispatch<"()">,
        auto (cref<fv3> pos) const noexcept -> f32
    >
    ::template add_convention<pro::operator_dispatch<"[]">,
        auto (cref<iv3> ijk) noexcept -> ref<f32>,
        auto (cref<iv3> ijk) const noexcept -> f32
    >
    ::template add_skill<pro::skills::as_view>
    ::build {};
}
