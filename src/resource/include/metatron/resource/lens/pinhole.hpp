#pragma once
#include <metatron/resource/lens/lens.hpp>

namespace mtt::photo {
	struct Pinhole_Lens final {
		Pinhole_Lens(f32 focal_distance) noexcept;
		auto sample(math::Vector<f32, 3> o, math::Vector<f32, 2> u) const noexcept -> std::optional<lens::Interaction>;
	
	private:
		f32 focal_distance;
	};
}

