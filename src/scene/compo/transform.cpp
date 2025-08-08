#include <metatron/scene/compo/transform.hpp>

namespace mtt::compo {
	auto to_transform(Transform const& t) -> math::Transform {
		return std::visit([](auto&& compo) {
			using T = std::decay_t<decltype(compo)>;
			if constexpr (std::is_same_v<T, compo::Local_Transform>) {
				auto translation = math::Matrix<f32, 4, 4>{
					{1.f, 0.f, 0.f, compo.translation[0]},
					{0.f, 1.f, 0.f, compo.translation[1]},
					{0.f, 0.f, 1.f, compo.translation[2]},
					{0.f, 0.f, 0.f, 1.f}
				};
				auto scaling = math::Matrix<f32, 4, 4>{
					compo.scaling[0], compo.scaling[1], compo.scaling[2], 1.f
				};
				auto rotation = math::Matrix<f32, 4, 4>{compo.rotation};
				return math::Transform{translation | rotation | scaling};
			} else if constexpr (std::is_same_v<T, compo::Matrix_Transform>) {
				return math::Transform{compo.matrix};
			}
		}, t);
	}
}
