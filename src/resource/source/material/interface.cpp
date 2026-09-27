#include <metatron/resource/material/interface.hpp>

namespace mtt::material {
    Interface_Material::Interface_Material(Descriptor const&) noexcept {}

    auto Interface_Material::sample(
        math::Context const& ctx,
        muldim::Coordinate const& coord
    ) const noexcept -> Interaction<bsdf::Interface_Bsdf> {
        return {
            .bsdf = bsdf::Interface_Bsdf{},
            .emission = fv4{0.f},
            .normal = {0.f, 0.f, 1.f},
            .degraded = false,
        };
    }

    auto Interface_Material::flags() const noexcept -> Flags {
        return Flags::interface;
    }
}
