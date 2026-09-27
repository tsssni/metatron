#pragma once
#include <metatron/resource/media/interaction.hpp>
#include <metatron/resource/volume/volume.hpp>
#include <metatron/core/math/distribution/exponential.hpp>

namespace mtt::media {
    struct Heterogeneous_Medium final {
        struct Descriptor final {
            phase::Henyey_Greenstein_Phase_Function phase;
            spectra::Spectrum sigma_a = spectra::Spectrum::entity("/spectrum/zero");
            spectra::Spectrum sigma_s = spectra::Spectrum::entity("/spectrum/zero");
            spectra::Spectrum sigma_e = spectra::Spectrum::entity("/spectrum/zero");
            uzv3 dimensions;
            volume::Volume density;
            f32 density_scale;
        };
        Heterogeneous_Medium(Descriptor const& desc) noexcept;
        Heterogeneous_Medium() noexcept = default;

        struct Iterator final {
            Iterator(
                Heterogeneous_Medium const& m,
                math::Context const& ctx,
                f32 t
            ) noexcept;
            auto march(f32 uu) noexcept -> Interaction<phase::Henyey_Greenstein_Phase_Function>;

        private:
            auto update_majorant(f32 t) noexcept -> void;
            auto update_transmittance(f32 t) noexcept -> void;

            Heterogeneous_Medium const* medium;

            fv4 sigma_a;
            fv4 sigma_s;
            fv4 sigma_e;
            fv4 sigma_maj;
            fv4 transmittance = {1.f};

            math::Ray r;

            iv3 cell;
            iv3 offset;

            f32 t_max;
            f32 t_cell;
            f32 t_boundary;
            f32 t_transmitted;
            f32 u;
        };

        auto begin(math::Context const& ctx, f32 t_max) const noexcept -> Iterator;

    private:
        friend Iterator;

        phase::Henyey_Greenstein_Phase_Function phase;
        spectra::Spectrum sigma_a;
        spectra::Spectrum sigma_s;
        spectra::Spectrum sigma_e;
        volume::Volume majorant;
        volume::Volume density;
        f32 density_scale;
    };
}
