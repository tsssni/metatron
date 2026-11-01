#pragma once
#include <metatron/core/math/bounding-box.hpp>

namespace mtt::render {

struct Uniform_Volume final {
    core::Bounding_Box bbox;
    iv3 dims;
    fv3 voxel_size;
    ft3 storage;
    Uniform_Volume(core::Bounding_Box const& bbox, uzv3 dimensions) noexcept;

    auto to_local(iv3 const& ijk) const noexcept -> fv3;
    auto to_index(fv3 const& pos) const noexcept -> iv3;
    auto dimensions() const noexcept -> uzv3;

    auto inside(iv3 const& pos) const noexcept -> bool;
    auto inside(fv3 const& pos) const noexcept -> bool;

    auto bounding_box() const noexcept -> core::Bounding_Box;
    auto bounding_box(fv3 const& pos) const noexcept -> core::Bounding_Box;
    auto bounding_box(iv3 const& ijk) const noexcept -> core::Bounding_Box;

    auto operator()(fv3 const& pos) const noexcept -> f32;
    auto operator[](iv3 const& ijk) const noexcept -> f32;
};

}
