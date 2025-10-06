#pragma once
#include <metatron/resource/media/medium.hpp>
#include <metatron/core/math/bounding-box.hpp>
#include <metatron/core/math/distribution/exponential.hpp>
#include <metatron/core/math/grid/uniform.hpp>
#include <metatron/core/stl/thread.hpp>
#include <nanovdb/NanoVDB.h>
#include <nanovdb/io/IO.h>
#include <nanovdb/math/SampleFromVoxels.h>
#include <nanovdb/math/Ray.h>
#include <string_view>

namespace mtt::media {
    template<typename T, usize n>
    auto to_nanovdb(math::Vector<T, n> x) {
        if constexpr (n == 3) {
            return nanovdb::math::Vec3<T>{x[0], x[1], -x[2]};
        }
    }

    template<typename T>
    auto from_nanovdb(nanovdb::math::Vec3<T> const& x) -> math::Vector<T, 3> {
        return {x[0], x[1], -x[2]};
    }

    auto inline from_nanovdb(nanovdb::Vec3dBBox const& bbox) {
        auto p_min = from_nanovdb(nanovdb::math::Vec3f{bbox.mCoord[0]});
        auto p_max = from_nanovdb(nanovdb::math::Vec3f{bbox.mCoord[1]});
        std::swap(p_min[2], p_max[2]);
        return math::Bounding_Box{p_min, p_max};
    }

    template<typename T>
    using Nanovdb_Sampler = nanovdb::math::SampleFromVoxels<nanovdb::NanoTree<T>, 1, false>;

    template<typename T>
    struct Nanovdb_Grid final {
        Nanovdb_Grid(
            std::string_view path,
            math::Vector<usize, 3> const& dimensions
        ) noexcept:
        handle(nanovdb::io::readGrid(std::string{path})),
        nanovdb_grid(handle.grid<T>()),
        sampler(nanovdb_grid->tree()),
        majorant_grid(from_nanovdb(nanovdb_grid->worldBBox()), dimensions),
        background(
            sampler(nanovdb_grid->worldToIndex(
                to_nanovdb(majorant_grid.bounding_box().p_min - 1.f)
            ))
        ) {
            auto& root = nanovdb_grid->tree().root();
            stl::scheduler::instance().sync_parallel(
                majorant_grid.dimensions(),
                [&](math::Vector<usize, 3> const& xyz) {
                    auto ijk = math::Vector<i32, 3>{xyz};
                    auto voxel_bbox = majorant_grid.bounding_box(ijk);
                    majorant_grid[ijk] = math::low<T>;

                    std::swap(voxel_bbox.p_min[2], voxel_bbox.p_max[2]);
                    auto p_min = math::Vector<i32, 3>{from_nanovdb(nanovdb_grid->worldToIndex(to_nanovdb(voxel_bbox.p_min)))};
                    auto p_max = math::Vector<i32, 3>{from_nanovdb(nanovdb_grid->worldToIndex(to_nanovdb(voxel_bbox.p_max)))};
                    p_min[2] *= -1;
                    p_max[2] *= -1;

                    for (auto i = p_min[0]; i <= p_max[0]; i++) {
                        for (auto j = p_min[1]; j <= p_max[1]; j++) {
                            for (auto k = p_min[2]; k <= p_max[2]; k++) {
                                majorant_grid[ijk] = std::max(majorant_grid[ijk], nanovdb_grid->tree().getValue({i, j, k}));
                            }
                        }
                    }
                }
            );
        }

        auto to_local(math::Vector<i32, 3> const& ijk) const noexcept -> math::Vector<f32, 3> { return majorant_grid.to_local(ijk); }
        auto to_index(math::Vector<f32, 3> const& pos) const noexcept -> math::Vector<i32, 3> { return majorant_grid.to_index(pos); }
        auto dimensions() const noexcept -> math::Vector<usize, 3> { return majorant_grid.dimensions(); }
        auto inside(math::Vector<i32, 3> const& pos) const noexcept -> bool { return majorant_grid.inside(pos); }
        auto inside(math::Vector<f32, 3> const& pos) const noexcept -> bool { return majorant_grid.inside(pos); }
        auto bounding_box() const noexcept -> math::Bounding_Box { return majorant_grid.bounding_box(); }
        auto bounding_box(math::Vector<f32, 3> const& pos) const noexcept -> math::Bounding_Box { return majorant_grid.bounding_box(pos); }
        auto bounding_box(math::Vector<i32, 3> const& ijk) const noexcept -> math::Bounding_Box { return majorant_grid.bounding_box(ijk); }

        auto operator()(math::Vector<f32, 3> const& pos) const noexcept -> T {
            return sampler(nanovdb_grid->worldToIndex(to_nanovdb(pos)));
        }
        auto operator[](math::Vector<i32, 3> const& ijk) noexcept -> T& {
            if (majorant_grid.inside(ijk)) {
                return majorant_grid[ijk];
            } else {
                return background;
            }
        }
        auto operator[](math::Vector<i32, 3> const& ijk) const noexcept -> T const& {
            return const_cast<Nanovdb_Grid&>(*this)[ijk];
        }

    private:
        nanovdb::GridHandle<> handle;
        nanovdb::Grid<nanovdb::NanoTree<T>> const* nanovdb_grid;
        Nanovdb_Sampler<T> sampler;
        math::Uniform_Grid<T> majorant_grid;
        T background;
    };
}
