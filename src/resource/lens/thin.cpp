#include <metatron/resource/lens/thin.hpp>
#include <metatron/core/math/distribution/disk.hpp>
#include <metatron/core/math/constant.hpp>

namespace mtt::photo {
    Thin_Lens::Thin_Lens(
        f32 aperture,
        f32 focal_length,
        f32 focus_distance
    ) noexcept:
    aperture(aperture),
    focal_length(focal_length),
    focus_distance(focus_distance),
    focal_distance(1.f / (1.f / focal_length - 1.f / focus_distance)) {}

    auto Thin_Lens::sample(math::Vector<f32, 2> o, math::Vector<f32, 2> u) const noexcept -> std::optional<lens::Interaction> {
        auto center = math::Vector<f32, 3>{0.f};
        auto focused = math::expand(-o, focal_distance) * focus_distance / focal_distance;
        
        auto radius = focal_distance / aperture / 2.f;
        auto distr = math::Unifrom_Disk_Distribution{};
        auto lens_p = math::Vector<f32, 3>{distr.sample(u) * radius, 0};
        auto direction = math::normalize(focused - lens_p);

        return lens::Interaction{{lens_p, direction}, distr.pdf()};
    }
}
