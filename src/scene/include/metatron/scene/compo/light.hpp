#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <variant>

namespace mtt::compo {
	struct Parallel_Light final {
		std::string_view parallel = "";
		ecs::Entity spectrum;
	};

	struct Point_Light final {
		std::string_view point = "";
		ecs::Entity spectrum;
	};

	struct Spot_Light final {
		std::string_view spot = "";
		ecs::Entity spectrum;
		f32 falloff_start_theta;
		f32 falloff_end_theta;
	};

	struct Environment_Light final {
		std::string_view environment = "";
		ecs::Entity env_map;
	};

	using Light = std::variant<
		Parallel_Light,
		Point_Light,
		Spot_Light,
		Environment_Light
	>;
}
