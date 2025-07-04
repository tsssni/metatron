#include <metatron/render/lens/pinhole.hpp>
#include <metatron/core/math/constant.hpp>

namespace metatron::photo {
	Pinhole_Lens::Pinhole_Lens(f32 focal_length)
		: focal_length(focal_length) {}

	auto Pinhole_Lens::sample(math::Vector<f32, 3> o, math::Vector<f32, 2> u) -> std::optional<lens::Interaction> {
		auto p = math::Vector<f32, 3>{0.f, 0.f, focal_length};
		return lens::Interaction{{p, math::normalize(p - o)}, 1.f};
	}
}
