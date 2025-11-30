#pragma once
#include <metatron/resource/volume/volume.hpp>
#include <metatron/resource/opaque/grid.hpp>
#include <metatron/core/stl/vector.hpp>

namespace mtt::volume {
    struct Uniform_Volume final {
        struct Descriptor final {
            math::Bounding_Box bbox;
            uzv3 dimensions;
        };
        Uniform_Volume() noexcept = default;
        Uniform_Volume(cref<Descriptor> desc) noexcept;

        auto to_local(cref<iv3> ijk) const noexcept -> fv3;
        auto to_index(cref<fv3> pos) const noexcept -> iv3;
        auto dimensions() const noexcept -> uzv3;

        auto inside(cref<iv3> pos) const noexcept -> bool;
        auto inside(cref<fv3> pos) const noexcept -> bool;

        auto bounding_box() const noexcept -> math::Bounding_Box;
        auto bounding_box(cref<fv3> pos) const noexcept -> math::Bounding_Box;
        auto bounding_box(cref<iv3> ijk) const noexcept -> math::Bounding_Box;

        auto operator()(cref<fv3> pos) const noexcept -> f32;
        auto operator[](cref<iv3> ijk) noexcept -> ref<f32>;
        auto operator[](cref<iv3> ijk) const noexcept -> f32;

    private:
        math::Bounding_Box bbox;
        iv3 dims;
        fv3 voxel_size;
        tag<opaque::Grid> storage;
    };
}
