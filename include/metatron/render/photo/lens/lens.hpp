#pragma once
#include <metatron/render/photo/lens/pinhole.hpp>
#include <metatron/render/photo/lens/thin.hpp>
#include <metatron/core/stl/protocol.hpp>

namespace mtt::photo {
    namespace lens {
        auto init() noexcept -> void;
    }

    struct Lens final: stl::polynomial<Lens
    , Pinhole_Lens
    , Thin_Lens> {
        using polynomial::polynomial;

        auto sample(cref<fv2> o, cref<fv2> u) const noexcept -> lens::Interaction {
            return visit([&](auto* p) noexcept { return p->sample(o, u); });
        }
    };
}
