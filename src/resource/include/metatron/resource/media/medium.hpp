#pragma once
#include <metatron/resource/media/interaction.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/math/eval.hpp>
#include <metatron/core/stl/protocol.hpp>

namespace mtt::media {
    struct Homogeneous_Medium;
    struct Heterogeneous_Medium;
    struct Vaccum_Medium;
    struct Iterator;
}

#include <metatron/resource/media/homogeneous.hpp>
#include <metatron/resource/media/heterogeneous.hpp>
#include <metatron/resource/media/vaccum.hpp>

namespace mtt::media {
    struct Iterator final: stl::variant<
        Iterator,
        Homogeneous_Medium::Iterator,
        Heterogeneous_Medium::Iterator,
        Vaccum_Medium::Iterator
    > {
        using variant::variant;

        auto march(f32 u) noexcept -> opt<Interaction> {
            return visit([u](auto* p) noexcept { return p->march(u); });
        }
    };

    struct Medium final: stl::polynomial<
        Medium,
        Homogeneous_Medium,
        Heterogeneous_Medium,
        Vaccum_Medium
    > {
        using polynomial::polynomial;
        auto static init() noexcept -> void;

        auto begin(cref<math::Context> ctx, f32 t_max) const noexcept -> Iterator {
            return visit([&, t_max](auto* p) noexcept { return Iterator{p->begin(ctx, t_max)}; });
        }
    };
}
