#include <metatron/render/lens/thin.hpp>
#include <metatron/core/math/distribution/disk.hpp>
#include <metatron/core/math/constant.hpp>

namespace mtt::photo {
	Thin_Lens::Thin_Lens(
		f32 aperture,
		f32 focal_length,
		f32 focus_distance
	):
	aperture(aperture),
	focal_length(focal_length),
	focus_distance(focus_distance) {}

	auto Thin_Lens::sample(math::Vector<f32, 3> o, math::Vector<f32, 2> u) -> std::optional<lens::Interaction> {
		auto center = math::Vector<f32, 3>{0.f, 0.f, focal_length};
		auto focused = center + (center - o) * focus_distance / focal_length;
		
		auto radius = focal_length / aperture / 2.f;
		auto distr = math::Unifrom_Disk_Distribution{};
		auto lens_p = math::Vector<f32, 3>{distr.sample(u) * radius, focal_length};
		auto direction = math::normalize(focused - lens_p);

		return lens::Interaction{{lens_p, direction}, distr.pdf()};
	}
}
