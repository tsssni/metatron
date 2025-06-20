#pragma once
#include <string_view>
#include <metatron/resource/shape/mesh.hpp>
#include <metatron/resource/material/material.hpp>

namespace metatron::loader {
	using Asset = std::tuple<
		std::unique_ptr<shape::Mesh>,
		std::unique_ptr<material::Material>
	>;
	struct Loader {
		auto virtual from_path(std::string_view path) -> std::vector<std::unique_ptr<shape::Mesh>> = 0;
	};
}
