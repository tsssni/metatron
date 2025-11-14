#include <metatron/resource/material/interface.hpp>
#include <metatron/resource/bsdf/interface.hpp>

namespace mtt::material {
    auto Interface_Material::sample(
        cref<eval::Context> ctx,
        cref<image::Coordinate> coord
    ) const noexcept -> opt<Interaction> {
        return Interaction{
            .bsdf = make_obj<bsdf::Bsdf, bsdf::Interface_Bsdf>(ctx.spec),
            .emission = ctx.spec & spectra::Spectrum::spectra["zero"],
            .normal = {0.f, 0.f, 1.f},
            .degraded = false,
        };
    }

    auto Interface_Material::flags() const noexcept -> Flags {
        return Flags::interface;
    }
}
