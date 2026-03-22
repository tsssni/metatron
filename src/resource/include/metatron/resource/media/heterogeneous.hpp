#pragma once
#include <metatron/resource/media/medium.hpp>
#include <metatron/resource/volume/volume.hpp>
#include <metatron/core/math/bounding-box.hpp>
#include <metatron/core/math/distribution/exponential.hpp>

namespace mtt::media {
    struct Heterogeneous_Medium final {
        struct Descriptor final {
            Phase phase;
            tag<spectra::Spectrum> sigma_a = entity<spectra::Spectrum>("/spectrum/zero");
            tag<spectra::Spectrum> sigma_s = entity<spectra::Spectrum>("/spectrum/zero");
            tag<spectra::Spectrum> sigma_e = entity<spectra::Spectrum>("/spectrum/zero");
            uzv3 dimensions;
            tag<volume::Volume> density;
            f32 density_scale;
        };
        Heterogeneous_Medium(cref<Descriptor> desc) noexcept;
        Heterogeneous_Medium() noexcept = default;

        struct Iterator final {
            Iterator(
                cref<Heterogeneous_Medium> m,
                cref<math::Context> ctx,
                f32 t
            ) noexcept;
            auto march(f32 uu) noexcept -> opt<Interaction>;

        private:
            auto update_majorant(f32 t) noexcept -> void;
            auto update_transmittance(f32 t) noexcept -> void;

            Heterogeneous_Medium const* medium;
            Interaction intr = {};

            fv4 sigma_a;
            fv4 sigma_s;
            fv4 sigma_t;
            fv4 sigma_maj;

            fv4 lambda;
            fv4 transmittance = {1.f};
            f32 density_maj = 0.f;
            math::Exponential_Distribution distr = {0.f};

            math::Ray r;
            f32 u;

            iv3 cell;
            iv3 offset;
            iv3 direction;

            f32 t_max;
            f32 t_cell;
            f32 t_boundary;
            f32 t_transmitted;
        };

        auto begin(cref<math::Context> ctx, f32 t_max) const noexcept -> obj<media::Iterator>;

    private:
        friend Iterator;

        Phase phase;
        tag<spectra::Spectrum> sigma_a;
        tag<spectra::Spectrum> sigma_s;
        tag<spectra::Spectrum> sigma_e;
        tag<volume::Volume> majorant;
        tag<volume::Volume> density;
        f32 density_scale;
    };
}
