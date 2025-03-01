#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/ray.hpp>

namespace metatron::photo {
	struct Lens {
		virtual ~Lens();
		auto virtual sample(math::Vector<f32, 3> o, math::Vector<f32, 2> u) -> math::Ray = 0;
	};

	struct Pinhole_Lens final: Lens {
		Pinhole_Lens(f32 focal_length);
		auto sample(math::Vector<f32, 3> o, math::Vector<f32, 2> u) -> math::Ray;
	
	private:
		f32 focal_length;
	};
}
