#include <metatron/resource/lens/pinhole.hpp>
#include <metatron/core/math/constant.hpp>

namespace mtt::photo {
	Pinhole_Lens::Pinhole_Lens(f32 focal_distance) noexcept
	: focal_distance(focal_distance) {}

	auto Pinhole_Lens::sample(math::Vector<f32, 3> o, math::Vector<f32, 2> u) const noexcept -> std::optional<lens::Interaction> {
		auto p = math::Vector<f32, 3>{0.f, 0.f, focal_distance};
		return lens::Interaction{{p, math::normalize(p - o)}, 1.f};
	}
}
