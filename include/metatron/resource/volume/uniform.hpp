#pragma once
#include <metatron/resource/muldim/grid.hpp>
#include <metatron/core/math/bounding-box.hpp>

namespace mtt::volume {
    struct Uniform_Volume final {
        struct Descriptor final {
            math::Bounding_Box bbox;
            uzv3 dimensions;
        };
        Uniform_Volume(Descriptor const& desc) noexcept;
        Uniform_Volume() noexcept = default;

        auto to_local(iv3 const& ijk) const noexcept -> fv3;
        auto to_index(fv3 const& pos) const noexcept -> iv3;
        auto dimensions() const noexcept -> uzv3;

        auto inside(iv3 const& pos) const noexcept -> bool;
        auto inside(fv3 const& pos) const noexcept -> bool;

        auto bounding_box() const noexcept -> math::Bounding_Box;
        auto bounding_box(fv3 const& pos) const noexcept -> math::Bounding_Box;
        auto bounding_box(iv3 const& ijk) const noexcept -> math::Bounding_Box;

        auto operator()(fv3 const& pos) const noexcept -> f32;
        auto operator[](iv3 const& ijk) noexcept -> f32&;
        auto operator[](iv3 const& ijk) const noexcept -> f32;

    private:
        math::Bounding_Box bbox;
        iv3 dims;
        fv3 voxel_size;
        muldim::proxy::Grid storage;
    };
}
