#pragma once
#include <metatron/resource/loader/loader.hpp>
#include <assimp/scene.h>

namespace mtt::loader {
	struct Assimp_Loader final {
		auto from_path(
			std::string_view path
		) noexcept -> std::vector<poly<shape::Mesh>>;
	
	private:
		auto traverse(aiNode const* node) noexcept -> void;
		auto load_mesh(aiMesh const* mesh) noexcept -> void;
		auto load_material(aiMaterial const* material) noexcept -> void;

		aiScene const* scene{};
		std::vector<poly<shape::Mesh>> meshes{};
	};
}
