#include <metatron/resource/volume/nanovdb.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/optional.hpp>
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
    auto from_nanovdb(nanovdb::math::Vec3<T> const& x) -> math::Vector<T, 3> {
        return {x[0], x[1], -x[2]};
    }

    auto from_nanovdb(nanovdb::math::Coord const& x) -> math::Vector<i32, 3> {
        return {x.x(), x.y(), -x.z()};
    }

    auto from_nanovdb(nanovdb::Vec3dBBox const& bbox) {
        auto p_min = from_nanovdb(nanovdb::math::Vec3f{bbox.min()});
        auto p_max = from_nanovdb(nanovdb::math::Vec3f{bbox.max()});
        std::swap(p_min[2], p_max[2]);
        return math::Bounding_Box{p_min, p_max};
    }

    auto from_nanovdb(nanovdb::CoordBBox const& bbox) {
        auto p_min = from_nanovdb(bbox.min());
        auto p_max = from_nanovdb(bbox.max());
        std::swap(p_min[2], p_max[2]);
        return math::Bounding_Box{p_min, p_max};
    }

    Nanovdb_Volume::Nanovdb_Volume(Descriptor const& desc) noexcept {
        MTT_OPT_OR_CALLBACK(path, stl::filesystem::instance().find(desc.path), {
            std::println("volume {} not exists", desc.path);
            std::abort();
        });
        auto& vec = stl::vector<nanovdb::GridHandle<>>::instance();
        auto lock = vec.lock();
        handle = vec.push_back(nanovdb::io::readGrid(path));
    }

    auto Nanovdb_Volume::to_local(math::Vector<i32, 3> const& ijk) const noexcept -> math::Vector<f32, 3> {
        return from_nanovdb(grid()->indexToWorld(to_nanovdb(math::Vector<f32, 3>{ijk})));
    };

    auto Nanovdb_Volume::to_index(math::Vector<f32, 3> const& pos) const noexcept -> math::Vector<i32, 3> {
        return from_nanovdb(grid()->worldToIndex(to_nanovdb(pos)).floor());
    };

    auto Nanovdb_Volume::dimensions() const noexcept -> math::Vector<usize, 3> {
        auto bbox = from_nanovdb(grid()->indexBBox());
        return (bbox.p_max - bbox.p_min) + 1;
    }

    auto Nanovdb_Volume::inside(math::Vector<i32, 3> const& pos) const noexcept -> bool {
        auto bbox = from_nanovdb(grid()->indexBBox());
        auto fpos = math::Vector<f32, 3>{pos};
        return fpos >= bbox.p_min && fpos <= bbox.p_max;
    }

    auto Nanovdb_Volume::inside(math::Vector<f32, 3> const& pos) const noexcept -> bool {
        return inside(to_index(pos));
    }

    auto Nanovdb_Volume::bounding_box() const noexcept -> math::Bounding_Box {
        return from_nanovdb(grid()->worldBBox());
    }

    auto Nanovdb_Volume::bounding_box(math::Vector<f32, 3> const& pos) const noexcept -> math::Bounding_Box {
        return bounding_box(to_index(pos));
    }

    auto Nanovdb_Volume::bounding_box(math::Vector<i32, 3> const& ijk) const noexcept -> math::Bounding_Box {
        if (inside(ijk)) {
            return {to_local(ijk), to_local(ijk + 1)};
        } else {
            return bounding_box();
        }
    }

    auto Nanovdb_Volume::operator()(math::Vector<f32, 3> const& pos) const noexcept -> f32 {
        using Sampler = nanovdb::math::SampleFromVoxels<nanovdb::NanoTree<f32>, 1, false>;
        return Sampler(grid()->tree())(grid()->worldToIndex(to_nanovdb(pos)));
    }

    auto Nanovdb_Volume::operator[](math::Vector<i32, 3> const& ijk) noexcept -> f32& {
        std::println("nanovdb is readonly");
        std::abort();
    }

    auto Nanovdb_Volume::operator[](math::Vector<i32, 3> const& ijk) const noexcept -> f32 {
        return grid()->tree().getValue(to_nanovdb(ijk));
    }

    auto Nanovdb_Volume::grid() const -> view<nanovdb::FloatGrid> {
        return handle->grid<f32>();
    }
}
