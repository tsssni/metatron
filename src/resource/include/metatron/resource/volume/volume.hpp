#pragma once
#include <metatron/resource/volume/uniform.hpp>
#include <metatron/resource/volume/nanovdb.hpp>
#include <metatron/core/stl/protocol.hpp>

namespace mtt::volume {
    struct Volume final: stl::polynomial<Volume, Uniform_Volume, Nanovdb_Volume> {
        using polynomial::polynomial;
        auto static init() noexcept -> void;

        auto to_local(cref<iv3> ijk) const noexcept -> fv3 {
            return visit([&](auto* p) noexcept { return p->to_local(ijk); });
        }
        auto to_index(cref<fv3> pos) const noexcept -> iv3 {
            return visit([&](auto* p) noexcept { return p->to_index(pos); });
        }
        auto dimensions() const noexcept -> uzv3 {
            return visit([&](auto* p) noexcept { return p->dimensions(); });
        }
        auto inside(cref<iv3> pos) const noexcept -> bool {
            return visit([&](auto* p) noexcept { return p->inside(pos); });
        }
        auto inside(cref<fv3> pos) const noexcept -> bool {
            return visit([&](auto* p) noexcept { return p->inside(pos); });
        }
        auto bounding_box() const noexcept -> math::Bounding_Box {
            return visit([&](auto* p) noexcept { return p->bounding_box(); });
        }
        auto bounding_box(cref<fv3> pos) const noexcept -> math::Bounding_Box {
            return visit([&](auto* p) noexcept { return p->bounding_box(pos); });
        }
        auto bounding_box(cref<iv3> ijk) const noexcept -> math::Bounding_Box {
            return visit([&](auto* p) noexcept { return p->bounding_box(ijk); });
        }
        auto operator()(cref<fv3> pos) const noexcept -> f32 {
            return visit([&](auto* p) noexcept { return (*p)(pos); });
        }
        auto operator[](cref<iv3> ijk) noexcept -> ref<f32> {
            return visit([&](auto* p) noexcept -> ref<f32> { return (*p)[ijk]; });
        }
        auto operator[](cref<iv3> ijk) const noexcept -> f32 {
            return visit([&](auto* p) noexcept { return (*p)[ijk]; });
        }
    };
}
