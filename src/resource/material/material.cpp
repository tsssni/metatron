#include <metatron/resource/material/material.hpp>
#include <metatron/resource/bsdf/interface.hpp>
#include <metatron/resource/spectra/constant.hpp>

namespace mtt::material {
    auto Material::sample(
        eval::Context const& ctx,
        device::Coordinate const& coord
    ) const noexcept -> std::optional<Interaction> {
        auto attr = bsdf::Attribute{};
        auto intr = Interaction{};
        auto spec = ctx.spec;

        auto null_spec = spec & spectra::Spectrum::spectra["zero"].data();
        auto geometry_normal = math::Vector<f32, 3>{0.5f, 0.5f, 1.f};
        attr.spectra["spectrum"] = null_spec;
        attr.spectra["emission"] = null_spec;
        attr.vectors["normal"] = geometry_normal;

        for (auto const& [name, tex]: spectrum_textures)
            attr.spectra[name] = (*tex)(coord, spec);
        for (auto const& [name, tex]: vector_textures)
            attr.vectors[name] = (*tex)(coord);

        intr.emission = attr.spectra["emission"];
        intr.normal = math::normalize(math::shrink(attr.vectors["normal"]) * 2.f - 1.f);

        attr.inside = ctx.inside;
        intr.bsdf = configurator(attr);
        intr.degraded = intr.bsdf->degrade();

        return intr;
    }
}
