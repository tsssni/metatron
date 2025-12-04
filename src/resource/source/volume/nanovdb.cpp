#include <metatron/resource/volume/nanovdb.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/print.hpp>
#include <nanovdb/math/SampleFromVoxels.h>

namespace mtt::volume {
    template<typename T, usize n>
    auto to_nanovdb(math::Vector<T, n> x) {
        if constexpr (n == 3) {
            if constexpr (std::floating_point<T>)
                return nanovdb::math::Vec3<T>{x[0], x[1], -x[2]};
            else if constexpr (std::integral<T>)
                return nanovdb::math::Coord{x[0], x[1], -x[2]};
        }
    }

    template<typename T>
    auto from_nanovdb(cref<nanovdb::math::Vec3<T>> x) -> math::Vector<T, 3> {
        return {x[0], x[1], -x[2]};
    }

    auto from_nanovdb(cref<nanovdb::math::Coord> x) -> iv3 {
        return {x.x(), x.y(), -x.z()};
    }

    auto from_nanovdb(cref<nanovdb::Vec3dBBox> bbox) {
        auto p_min = from_nanovdb(nanovdb::math::Vec3f{bbox.min()});
        auto p_max = from_nanovdb(nanovdb::math::Vec3f{bbox.max()});
        std::swap(p_min[2], p_max[2]);
        return math::Bounding_Box{p_min, p_max};
    }

    auto from_nanovdb(cref<nanovdb::CoordBBox> bbox) {
        auto p_min = from_nanovdb(bbox.min());
        auto p_max = from_nanovdb(bbox.max());
        std::swap(p_min[2], p_max[2]);
        return math::Bounding_Box{p_min, p_max};
    }

    Nanovdb_Volume::Nanovdb_Volume(cref<Descriptor> desc) noexcept {
        auto path = stl::filesystem::find(desc.path);
        auto& vec = stl::vector<nanovdb::GridHandle<>>::instance();
        auto lock = vec.lock();
        handle = vec.push_back(nanovdb::io::readGrid(path));
    }

    auto Nanovdb_Volume::to_local(cref<iv3> ijk) const noexcept -> fv3 {
        return from_nanovdb(grid()->indexToWorld(to_nanovdb(fv3{ijk})));
    };

    auto Nanovdb_Volume::to_index(cref<fv3> pos) const noexcept -> iv3 {
        return from_nanovdb(grid()->worldToIndex(to_nanovdb(pos)).floor());
    };

    auto Nanovdb_Volume::dimensions() const noexcept -> uzv3 {
        auto bbox = from_nanovdb(grid()->indexBBox());
        return (bbox.p_max - bbox.p_min) + 1;
    }

    auto Nanovdb_Volume::inside(cref<iv3> pos) const noexcept -> bool {
        auto bbox = from_nanovdb(grid()->indexBBox());
        auto fpos = fv3{pos};
        return fpos >= bbox.p_min && fpos <= bbox.p_max;
    }

    auto Nanovdb_Volume::inside(cref<fv3> pos) const noexcept -> bool {
        return inside(to_index(pos));
    }

    auto Nanovdb_Volume::bounding_box() const noexcept -> math::Bounding_Box {
        return from_nanovdb(grid()->worldBBox());
    }

    auto Nanovdb_Volume::bounding_box(cref<fv3> pos) const noexcept -> math::Bounding_Box {
        return bounding_box(to_index(pos));
    }

    auto Nanovdb_Volume::bounding_box(cref<iv3> ijk) const noexcept -> math::Bounding_Box {
        if (inside(ijk)) {
            return {to_local(ijk), to_local(ijk + 1)};
        } else {
            return bounding_box();
        }
    }

    auto Nanovdb_Volume::operator()(cref<fv3> pos) const noexcept -> f32 {
        using Sampler = nanovdb::math::SampleFromVoxels<nanovdb::NanoTree<f32>, 1, false>;
        return Sampler(grid()->tree())(grid()->worldToIndex(to_nanovdb(pos)));
    }

    auto Nanovdb_Volume::operator[](cref<iv3> ijk) noexcept -> ref<f32> {
        stl::abort("nanovdb is readonly");
        return *(mut<f32>)handle.data();
    }

    auto Nanovdb_Volume::operator[](cref<iv3> ijk) const noexcept -> f32 {
        return grid()->tree().getValue(to_nanovdb(ijk));
    }

    auto Nanovdb_Volume::grid() const -> view<nanovdb::FloatGrid> {
        return handle->grid<f32>();
    }
}
