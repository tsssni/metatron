#pragma once
#include <metatron/render/lens/lens.hpp>

namespace metatron::photo {
	struct Pinhole_Lens final: Lens {
		Pinhole_Lens(f32 focal_length);
		auto sample(math::Vector<f32, 3> o, math::Vector<f32, 2> u) -> std::optional<lens::Interaction>;
	
	private:
		f32 focal_length;
	};
}

