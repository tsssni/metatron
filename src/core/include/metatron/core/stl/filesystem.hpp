#pragma once
#include <metatron/core/stl/singleton.hpp>
#include <vector>

namespace mtt::stl {
	struct filesystem final: stl::singleton<filesystem> {
		filesystem();

		auto push(std::string const& path) noexcept -> void;
		auto pop(std::string const& path) noexcept -> void;
		auto find(std::string const& path) noexcept -> std::optional<std::string>;

	private:
		auto push_env(std::string const& env) noexcept -> void;

		std::vector<std::string> pathes;
	};
}
