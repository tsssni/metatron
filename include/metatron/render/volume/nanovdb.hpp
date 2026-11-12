#pragma once
#include <metatron/core/infra/command.hpp>
#include <metatron/core/math/bounding-box.hpp>
#include <nanovdb/io/IO.h>
#include <string_view>

namespace mtt::render {

struct [[mtt::reflection(recorder)]] Nanovdb_Volume final {
    using Grid = nanovdb::GridHandle<>;
    Grid handle;
    core::Bounding_Box bbox;
    Nanovdb_Volume(std::string_view path, core::recorder& recorder) noexcept;

    auto to_local(iv3 const& ijk) const noexcept -> fv3;
    auto to_index(fv3 const& pos) const noexcept -> iv3;
    auto dimensions() const noexcept -> uzv3;

    auto inside(iv3 const& pos) const noexcept -> bool;
    auto inside(fv3 const& pos) const noexcept -> bool;

    auto bounding_box() const noexcept -> core::Bounding_Box;
    auto bounding_box(fv3 const& pos) const noexcept -> core::Bounding_Box;
    auto bounding_box(iv3 const& ijk) const noexcept -> core::Bounding_Box;

    auto operator()(fv3 const& pos) const noexcept -> f32;
    auto operator[](iv3 const& ijk) noexcept -> f32&;
    auto operator[](iv3 const& ijk) const noexcept -> f32;

private:
    auto grid() const -> nanovdb::FloatGrid const*;
};

}
