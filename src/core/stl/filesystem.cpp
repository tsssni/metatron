#include <metatron/core/stl/filesystem.hpp>
#include <cstdlib>
#include <sstream>
#include <ranges>
#include <filesystem>

namespace mtt::stl {
	filesystem::filesystem() {
		pathes.reserve(128);
		#if defined(__WIN32)
			push_env("APPDATA");
			push_env("LOCALAPPDATA");
		#else
			push_env("XDG_DATA_DIRS");
			push_env("XDG_DATA_HOME");
		#endif
		push_env("MTT_DATA");
	}

	auto filesystem::push(std::string_view path) noexcept -> void {
		pathes.push_back(std::string{path});
	}

	auto filesystem::pop(std::string_view path) noexcept -> void {
		pathes.pop_back();
	}

	auto filesystem::find(std::string_view path) noexcept -> std::optional<std::string> {
		for (const auto& base : pathes | std::views::reverse) {
			auto full = std::filesystem::path(base) / path;
			if (std::filesystem::exists(full)) {
				return full.string();
			}
		}
		return {};
	}

	auto filesystem::push_env(std::string_view env) noexcept -> void {
		#if defined(__WIN32)
			auto constexpr sep = ';';
		#else
			auto constexpr sep = ':';
		#endif

		auto env_value = std::getenv(env.data());
		if (!env_value) return;

		auto ss = std::stringstream{env_value};
		auto path = std::string{};
		auto pathes = std::vector<std::string>{};
		
		while(std::getline(ss, path, sep)) {
			if (!path.empty()) {
				pathes.push_back(path);
			}
		}

		for (auto path: pathes | std::views::reverse) {
			this->pathes.push_back(path);
		}
	}
}
