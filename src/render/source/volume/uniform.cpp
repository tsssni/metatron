#include <metatron/render/volume/uniform.hpp>

namespace mtt::render {

Uniform_Volume::Uniform_Volume(core::Bounding_Box const& bbox, uzv3 dimensions) noexcept
: bbox(bbox), dims(dimensions), voxel_size((bbox.p_max - bbox.p_min) / dims) {
    storage.update<core::ordinary<32>, core::encoding::sf>({dimensions, 1});
}

auto Uniform_Volume::to_local(iv3 const& ijk) const noexcept -> fv3 { return bbox.p_min + ijk * voxel_size; }
auto Uniform_Volume::to_index(fv3 const& pos) const noexcept -> iv3 { return core::floor((pos - bbox.p_min) / voxel_size); }
auto Uniform_Volume::dimensions() const noexcept -> uzv3 { return dims; }

auto Uniform_Volume::inside(iv3 const& pos) const noexcept -> bool {
    return core::all([](i32 p, i32 q, auto) { return p >= 0 && p < q; }, pos, dims);
}
auto Uniform_Volume::inside(fv3 const& pos) const noexcept -> bool { return inside(to_index(pos)); }

auto Uniform_Volume::bounding_box() const noexcept -> core::Bounding_Box { return bbox; }
auto Uniform_Volume::bounding_box(fv3 const& pos) const noexcept -> core::Bounding_Box { return bounding_box(to_index(pos)); }
auto Uniform_Volume::bounding_box(iv3 const& ijk) const noexcept -> core::Bounding_Box {
    if (ijk == clamp(ijk, iv3{0}, dims - 1)) return {
        bbox.p_min + fv3(ijk + 0) * voxel_size,
        bbox.p_min + fv3(ijk + 1) * voxel_size,
    };
    else return bbox;
}

auto Uniform_Volume::operator()(fv3 const& pos) const noexcept -> f32 {
    return (*this)[to_index(pos)];
}
auto Uniform_Volume::operator[](iv3 const& ijk) const noexcept -> f32 {
    return storage[uzv3{ijk}][0];
}

}
