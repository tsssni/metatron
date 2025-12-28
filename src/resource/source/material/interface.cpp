#include <metatron/resource/material/interface.hpp>
#include <metatron/resource/bsdf/interface.hpp>

namespace mtt::material {
    Interface_Material::Interface_Material(cref<Descriptor>) noexcept {}

    auto Interface_Material::sample(
        cref<math::Context> ctx,
        cref<muldim::Coordinate> coord
    ) const noexcept -> opt<Interaction> {
        return Interaction{
            .bsdf = make_obj<bsdf::Bsdf, bsdf::Interface_Bsdf>(),
            .emission = fv4{0.f},
            .normal = {0.f, 0.f, 1.f},
            .degraded = false,
        };
    }

    auto Interface_Material::flags() const noexcept -> Flags {
        return Flags::interface;
    }
}
