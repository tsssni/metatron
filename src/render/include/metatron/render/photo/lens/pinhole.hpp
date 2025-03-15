#pragma once
#include <metatron/render/photo/lens/lens.hpp>

namespace metatron::photo {
	struct Pinhole_Lens final: Lens {
		Pinhole_Lens(f32 focal_length);
		auto operator()(math::Ray const& r) -> std::optional<lens::Interaction>;
		auto sample(math::Vector<f32, 3> o, math::Vector<f32, 2> u) -> std::optional<lens::Interaction>;
	
	private:
		f32 focal_length;
	};
}

