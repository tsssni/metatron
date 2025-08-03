#pragma once
#include <metatron/resource/lens/lens.hpp>

namespace mtt::photo {
	struct Thin_Lens final {
		Thin_Lens(
			f32 aperture,
			f32 focal_length,
			f32 focus_distance
		) noexcept;
		auto sample(math::Vector<f32, 3> o, math::Vector<f32, 2> u) const noexcept -> std::optional<lens::Interaction>;
	
	private:
		f32 aperture;
		f32 focal_length;
		f32 focus_distance;
	};
}

