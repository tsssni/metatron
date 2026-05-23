#pragma once
#include <metatron/render/photo/lens/interaction.hpp>
#include <metatron/render/photo/lens/pinhole.hpp>
#include <metatron/render/photo/lens/thin.hpp>
#include <metatron/core/stl/protocol.hpp>

namespace mtt::photo {
    struct Lens final: stl::polynomial<Lens, Pinhole_Lens, Thin_Lens> {
        using polynomial::polynomial;
        auto static init() noexcept -> void;

        auto sample(cref<fv2> o, cref<fv2> u) const noexcept -> opt<lens::Interaction> {
            return visit([&](auto* p) noexcept { return p->sample(o, u); });
        }
    };
}
