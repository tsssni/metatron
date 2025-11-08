#pragma once
#include <metatron/resource/volume/volume.hpp>
#include <metatron/core/stl/vector.hpp>
#include <nanovdb/io/IO.h>

namespace mtt::volume {
    struct Nanovdb_Volume final {
        Nanovdb_Volume(std::string_view path) noexcept;

        auto to_local(math::Vector<i32, 3> const& ijk) const noexcept -> math::Vector<f32, 3>;
        auto to_index(math::Vector<f32, 3> const& pos) const noexcept -> math::Vector<i32, 3>;
        auto dimensions() const noexcept -> math::Vector<usize, 3>;

        auto inside(math::Vector<i32, 3> const& pos) const noexcept -> bool;
        auto inside(math::Vector<f32, 3> const& pos) const noexcept -> bool;

        auto bounding_box() const noexcept -> math::Bounding_Box;
        auto bounding_box(math::Vector<f32, 3> const& pos) const noexcept -> math::Bounding_Box;
        auto bounding_box(math::Vector<i32, 3> const& ijk) const noexcept -> math::Bounding_Box;

        auto at(math::Vector<i32, 3> const& ijk) const noexcept -> f32;
        auto operator()(math::Vector<f32, 3> const& pos) const noexcept -> f32;
        auto operator[](math::Vector<i32, 3> const& ijk) noexcept -> f32&;
        auto operator[](math::Vector<i32, 3> const& ijk) const noexcept -> f32;

    private:
        auto grid() const -> view<nanovdb::FloatGrid>;
        proxy<nanovdb::GridHandle<>> handle;
    };
}
