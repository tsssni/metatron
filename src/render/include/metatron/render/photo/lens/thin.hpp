#pragma once
#include <metatron/render/photo/lens/lens.hpp>

namespace metatron::photo {
	struct Thin_Lens final: Lens {
		Thin_Lens(
			f32 aperture,
			f32 focal_length,
			f32 focus_distance
		);
		auto sample(math::Vector<f32, 3> o, math::Vector<f32, 2> u) -> std::optional<lens::Interaction>;
	
	private:
		f32 aperture;
		f32 focal_length;
		f32 focus_distance;
	};
}

