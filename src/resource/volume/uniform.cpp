#include <metatron/resource/volume/uniform.hpp>

namespace mtt::volume {
    Uniform_Volume::Uniform_Volume(
        math::Bounding_Box const& bbox,
        math::Vector<usize, 3> const& dimensions
    ) noexcept:
    bbox(bbox),
    dims(dimensions),
    voxel_size((bbox.p_max - bbox.p_min) / dimensions),
    data(math::prod(dimensions)) {}

    auto Uniform_Volume::to_local(math::Vector<i32, 3> const& ijk) const noexcept -> math::Vector<f32, 3> {
        return bbox.p_min + ijk * voxel_size;
    };

    auto Uniform_Volume::to_index(math::Vector<f32, 3> const& pos) const noexcept -> math::Vector<i32, 3> {
        return floor((pos - bbox.p_min) / voxel_size);
    };

    auto Uniform_Volume::dimensions() const noexcept -> math::Vector<usize, 3> {
        return dims;
    }

    auto Uniform_Volume::inside(math::Vector<i32, 3> const& pos) const noexcept -> bool {
        return all(
            [](i32 p, i32 q, auto){return p >= 0 && p < q;},
            pos, dims
        );
    }

    auto Uniform_Volume::inside(math::Vector<f32, 3> const& pos) const noexcept -> bool {
        return inside(to_index(pos));
    }

    auto Uniform_Volume::bounding_box() const noexcept -> math::Bounding_Box {
        return bbox;
    }

    auto Uniform_Volume::bounding_box(math::Vector<f32, 3> const& pos) const noexcept -> math::Bounding_Box {
        return bounding_box(to_index(pos));
    }

    auto Uniform_Volume::bounding_box(math::Vector<i32, 3> const& ijk) const noexcept -> math::Bounding_Box {
        if (ijk == clamp(ijk, math::Vector<i32, 3>{0}, dims - 1)) {
            return math::Bounding_Box{
                bbox.p_min + math::Vector<f32, 3>(ijk + 0) * voxel_size,
                bbox.p_min + math::Vector<f32, 3>(ijk + 1) * voxel_size,
            };
        } else {
            return bbox;
        }
    }

    auto Uniform_Volume::at(math::Vector<i32, 3> const& ijk) const noexcept -> f32 {
        return (*this)[ijk];
    }

    auto Uniform_Volume::operator()(math::Vector<f32, 3> const& pos) const noexcept -> f32 {
        return (*this)[to_index(pos)];
    }

    auto Uniform_Volume::operator[](math::Vector<i32, 3> const& ijk) noexcept -> f32& {
        auto [i, j, k] = ijk;
        auto offset = i * dims[1] * dims[2] + j * dims[2] + k;
        return data[offset];
    }

    auto Uniform_Volume::operator[](math::Vector<i32, 3> const& ijk) const noexcept -> f32 {
        return const_cast<Uniform_Volume&>(*this)[ijk];
    }
}
