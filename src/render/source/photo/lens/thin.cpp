#include <metatron/render/photo/lens/thin.hpp>
#include <metatron/core/math/distribution/disk.hpp>
#include <metatron/core/math/constant.hpp>

namespace mtt::photo {
    Thin_Lens::Thin_Lens(cref<Descriptor> desc) noexcept:
    aperture(desc.aperture),
    focal_length(desc.focal_length),
    focus_distance(desc.focus_distance),
    focal_distance(1.f / (1.f / focal_length - 1.f / focus_distance)) {}

    auto Thin_Lens::sample(cref<fv2> o, cref<fv2> u) const noexcept -> opt<lens::Interaction> {
        auto center = fv3{0.f};
        auto focused = math::expand(-o, focal_distance) * focus_distance / focal_distance;

        auto radius = focal_distance / aperture / 2.f;
        auto distr = math::Unifrom_Disk_Distribution{};
        auto lens_p = fv3{distr.sample(u) * radius, 0};
        auto direction = math::normalize(focused - lens_p);

        return lens::Interaction{{lens_p, direction}, 1.f};
    }
}
