#include <metatron/render/photo/lens/pinhole.hpp>
#include <metatron/core/math/constant.hpp>

namespace metatron::photo {
	Pinhole_Lens::Pinhole_Lens(f32 focal_length)
		: focal_length(focal_length) {}

	auto Pinhole_Lens::sample(math::Vector<f32, 3> o, math::Vector<f32, 2> u) -> std::optional<lens::Interaction> {
		return lens::Interaction{
			{
				o,
				math::normalize(math::Vector<f32, 3>{0.f, 0.f, focal_length} - o)
			},
			1.f,
		};
	}
}
