#include <metatron/render/photo/lens.hpp>

namespace metatron::photo {
	Lens::~Lens() {}

	Pinhole_Lens::Pinhole_Lens(f32 focal_length)
		: focal_length(focal_length) {}

	auto Pinhole_Lens::sample(math::Vector<f32, 3> o, math::Vector<f32, 2> u) -> math::Ray {
		return {
			o,
			math::normalize(math::Vector<f32, 3>{0.f, 0.f, focal_length} - o)
		};
	}
}
