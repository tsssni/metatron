#include <metatron/resource/volume/uniform.hpp>

namespace mtt::volume {
    Uniform_Volume::Uniform_Volume(cref<Descriptor> desc) noexcept:
    bbox(desc.bbox),
    dims(desc.dimensions),
    voxel_size((bbox.p_max - bbox.p_min) / dims) {
        auto img = image::Image{};
        img.size = fv4{math::shrink(dims), 1uz, sizeof(f32)};
        img.pixels.resize(dims[2]);
        for (auto i = 0; i < dims[2]; ++i)
            img.pixels[i].resize(math::prod(img.size));

        auto& vec = stl::vector<image::Image>::instance();
        auto lock = vec.lock();
        storage = vec.push_back(std::move(img));
    }

    auto Uniform_Volume::to_local(cref<iv3> ijk) const noexcept -> fv3 {
        return bbox.p_min + ijk * voxel_size;
    };

    auto Uniform_Volume::to_index(cref<fv3> pos) const noexcept -> iv3 {
        return floor((pos - bbox.p_min) / voxel_size);
    };

    auto Uniform_Volume::dimensions() const noexcept -> uzv3 {
        return dims;
    }

    auto Uniform_Volume::inside(cref<iv3> pos) const noexcept -> bool {
        return all(
            [](i32 p, i32 q, auto){return p >= 0 && p < q;},
            pos, dims
        );
    }

    auto Uniform_Volume::inside(cref<fv3> pos) const noexcept -> bool {
        return inside(to_index(pos));
    }

    auto Uniform_Volume::bounding_box() const noexcept -> math::Bounding_Box {
        return bbox;
    }

    auto Uniform_Volume::bounding_box(cref<fv3> pos) const noexcept -> math::Bounding_Box {
        return bounding_box(to_index(pos));
    }

    auto Uniform_Volume::bounding_box(cref<iv3> ijk) const noexcept -> math::Bounding_Box {
        if (ijk == clamp(ijk, iv3{0}, dims - 1)) {
            return math::Bounding_Box{
                bbox.p_min + fv3(ijk + 0) * voxel_size,
                bbox.p_min + fv3(ijk + 1) * voxel_size,
            };
        } else {
            return bbox;
        }
    }

    auto Uniform_Volume::operator()(cref<fv3> pos) const noexcept -> f32 {
        return (*this)[to_index(pos)];
    }

    auto Uniform_Volume::operator[](cref<iv3> ijk) noexcept -> ref<f32> {
        auto [i, j, k] = ijk;
        auto ptr = (*storage)[i, j, k].data();
        return *(mut<f32>)ptr;
    }

    auto Uniform_Volume::operator[](cref<iv3> ijk) const noexcept -> f32 {
        return const_cast<ref<Uniform_Volume>>(*this)[ijk];
    }
}
