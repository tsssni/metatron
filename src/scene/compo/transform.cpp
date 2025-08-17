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
			} else if constexpr (std::is_same_v<T, compo::Look_At_Transform>) {
				auto position = compo.position;
				auto forward = math::normalize(compo.look_at - compo.position);
				auto right = math::normalize(math::cross(compo.up, forward));
				auto up = math::normalize(math::cross(forward, right));
				return math::Transform{math::Matrix<f32, 4, 4>{
					{right[0], up[0], forward[0], position[0]},
					{right[1], up[1], forward[1], position[1]},
					{right[2], up[2], forward[2], position[2]},
					{0.f,      0.f,   0.f,        1.f,       },
				}};
			}
		}, t);
	}
}
