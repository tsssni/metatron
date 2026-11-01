#include <metatron/render/photo/thin.hpp>
#include <metatron/core/distribution/disk.hpp>

namespace mtt::render {

Thin_Lens::Thin_Lens(f32 aperture, f32 focal_length, f32 focus_distance) noexcept
: aperture(aperture), focal_length(focal_length), focus_distance(focus_distance), focal_distance(1.f / (1.f / focal_length - 1.f / focus_distance)) {}

auto Thin_Lens::sample(fv2 const& o, fv2 const& u) const noexcept -> render::Lens_Interaction {
    auto center = fv3{0.f};
    auto focused = core::expand(-o, focal_distance) * focus_distance / focal_distance;

    auto radius = focal_distance / aperture / 2.f;
    auto distr = core::Unifrom_Disk_Distribution{};
    auto lens_p = fv3{distr.sample(u) * radius, 0};
    auto direction = core::normalize(focused - lens_p);

    return {{lens_p, direction}, 1.f};
}

}
