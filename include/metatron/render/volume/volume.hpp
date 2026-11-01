#pragma once
#include <metatron/render/volume/nanovdb.hpp>
#include <metatron/render/volume/uniform.hpp>

namespace mtt::render {

struct Volume final : core::protocol<Uniform_Volume, Nanovdb_Volume> {
    using protocol::protocol;

    auto to_local(iv3 const& ijk) const noexcept -> fv3;
    auto to_index(fv3 const& pos) const noexcept -> iv3;
    auto dimensions() const noexcept -> uzv3;
    auto inside(iv3 const& pos) const noexcept -> bool;
    auto inside(fv3 const& pos) const noexcept -> bool;
    auto bounding_box() const noexcept -> core::Bounding_Box;
    auto bounding_box(fv3 const& pos) const noexcept -> core::Bounding_Box;
    auto bounding_box(iv3 const& ijk) const noexcept -> core::Bounding_Box;
    auto operator()(fv3 const& pos) const noexcept -> f32;
    auto operator[](iv3 const& ijk) const noexcept -> f32;
};

auto inline Volume::to_local(iv3 const& ijk) const noexcept -> fv3 {
    return visit([&](auto* p) noexcept { return p->to_local(ijk); });
}

auto inline Volume::to_index(fv3 const& pos) const noexcept -> iv3 {
    return visit([&](auto* p) noexcept { return p->to_index(pos); });
}

auto inline Volume::dimensions() const noexcept -> uzv3 {
    return visit([&](auto* p) noexcept { return p->dimensions(); });
}

auto inline Volume::inside(iv3 const& pos) const noexcept -> bool {
    return visit([&](auto* p) noexcept { return p->inside(pos); });
}

auto inline Volume::inside(fv3 const& pos) const noexcept -> bool {
    return visit([&](auto* p) noexcept { return p->inside(pos); });
}

auto inline Volume::bounding_box() const noexcept -> core::Bounding_Box {
    return visit([&](auto* p) noexcept { return p->bounding_box(); });
}

auto inline Volume::bounding_box(fv3 const& pos) const noexcept -> core::Bounding_Box {
    return visit([&](auto* p) noexcept { return p->bounding_box(pos); });
}

auto inline Volume::bounding_box(iv3 const& ijk) const noexcept -> core::Bounding_Box {
    return visit([&](auto* p) noexcept { return p->bounding_box(ijk); });
}

auto inline Volume::operator()(fv3 const& pos) const noexcept -> f32 {
    return visit([&](auto* p) noexcept { return (*p)(pos); });
}

auto inline Volume::operator[](iv3 const& ijk) const noexcept -> f32 {
    return visit([&](auto* p) noexcept { return (*p)[ijk]; });
}

}
