#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <metatron/scene/serde/serde.hpp>
#include <variant>

namespace mtt::compo {
	struct Henyey_Greenstein_Phase_Function final {
		std::string_view henyey_greenstein = "";
		f32 g;
	};

	using Phase_Function = std::variant<
		Henyey_Greenstein_Phase_Function
	>;

	struct Vaccum_Medium final {
		std::string_view vaccum = "";
	};
	
	struct Homogeneous_Medium final {
		std::string_view homogeneous = "";
		Phase_Function phase;
		ecs::Entity sigma_a;
		ecs::Entity sigma_s;
		ecs::Entity sigma_e;
	};

	struct Grid_Medium final {
		std::string_view grid = "";
		std::string path;
		Phase_Function phase;
		ecs::Entity sigma_a;
		ecs::Entity sigma_s;
		ecs::Entity sigma_e;
		f32 density_scale;
	};

	using Medium = std::variant<
		Vaccum_Medium,
		Homogeneous_Medium,
		Grid_Medium
	>;

	struct Medium_Instance final {
		ecs::Entity path;
	};
}
