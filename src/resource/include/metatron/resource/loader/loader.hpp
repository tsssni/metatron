#pragma once
#include <string_view>
#include <metatron/resource/shape/mesh.hpp>
#include <metatron/resource/material/material.hpp>

namespace mtt::loader {
	MTT_POLY_METHOD(loader_from_path, from_path);

	struct Loader final: pro::facade_builder
	::add_convention<loader_from_path, auto (
		std::string_view path
	) const noexcept -> std::vector<std::unique_ptr<shape::Mesh>>>
	::support<pro::skills::as_view>
	::build {};
}
