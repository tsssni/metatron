#pragma once
#include <metatron/core/math/bounding-box.hpp>
#include <nanovdb/io/IO.h>

namespace mtt::volume {
    struct Nanovdb_Volume final {
        using Grid = nanovdb::GridHandle<>;
        struct Descriptor final {
            std::string path;
        };
        Nanovdb_Volume(Descriptor const& desc) noexcept;
        Nanovdb_Volume() noexcept = default;

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
        auto grid() const -> nanovdb::FloatGrid const*;
        tag<Grid> handle;
        math::Bounding_Box bbox;
    };
}
