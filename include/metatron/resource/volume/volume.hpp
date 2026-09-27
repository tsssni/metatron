#pragma once
#include <metatron/resource/volume/uniform.hpp>
#include <metatron/resource/volume/nanovdb.hpp>

namespace mtt::volume {
    auto init() noexcept -> void;

    struct Volume final: stl::polymorph<Volume
    , Uniform_Volume
    , Nanovdb_Volume> {
        using polymorph::polymorph;

        auto to_local(iv3 const& ijk) const noexcept -> fv3 {
            return visit([&](auto* p) noexcept { return p->to_local(ijk); });
        }
        auto to_index(fv3 const& pos) const noexcept -> iv3 {
            return visit([&](auto* p) noexcept { return p->to_index(pos); });
        }
        auto dimensions() const noexcept -> uzv3 {
            return visit([&](auto* p) noexcept { return p->dimensions(); });
        }
        auto inside(iv3 const& pos) const noexcept -> bool {
            return visit([&](auto* p) noexcept { return p->inside(pos); });
        }
        auto inside(fv3 const& pos) const noexcept -> bool {
            return visit([&](auto* p) noexcept { return p->inside(pos); });
        }
        auto bounding_box() const noexcept -> math::Bounding_Box {
            return visit([&](auto* p) noexcept { return p->bounding_box(); });
        }
        auto bounding_box(fv3 const& pos) const noexcept -> math::Bounding_Box {
            return visit([&](auto* p) noexcept { return p->bounding_box(pos); });
        }
        auto bounding_box(iv3 const& ijk) const noexcept -> math::Bounding_Box {
            return visit([&](auto* p) noexcept { return p->bounding_box(ijk); });
        }
        auto operator()(fv3 const& pos) const noexcept -> f32 {
            return visit([&](auto* p) noexcept { return (*p)(pos); });
        }
        auto operator[](iv3 const& ijk) noexcept -> f32& {
            return visit([&](auto* p) noexcept -> f32& { return (*p)[ijk]; });
        }
        auto operator[](iv3 const& ijk) const noexcept -> f32 {
            return visit([&](auto* p) noexcept { return (*p)[ijk]; });
        }
    };
}
