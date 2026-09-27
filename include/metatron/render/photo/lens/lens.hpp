#pragma once
#include <metatron/render/photo/lens/pinhole.hpp>
#include <metatron/render/photo/lens/thin.hpp>
#include <metatron/core/stl/protocol.hpp>

namespace mtt::photo::lens {
    auto init() noexcept -> void;
}

namespace mtt::photo {
    struct Lens final: stl::polymorph<Lens
    , Pinhole_Lens
    , Thin_Lens> {
        using polymorph::polymorph;

        auto sample(fv2 const& o, fv2 const& u) const noexcept -> lens::Interaction {
            return visit([&](auto* p) noexcept { return p->sample(o, u); });
        }
    };
}
