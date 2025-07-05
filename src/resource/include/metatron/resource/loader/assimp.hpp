#pragma once
#include <metatron/resource/loader/loader.hpp>
#include <assimp/scene.h>

namespace mtt::loader {
	struct Assimp_Loader final: Loader {
		auto from_path(std::string_view path) -> std::vector<std::unique_ptr<shape::Mesh>>;
	
	private:
		auto traverse(aiNode const* node) -> void;
		auto load_mesh(aiMesh const* mesh) -> void;
		auto load_material(aiMaterial const* material) -> void;

		aiScene const* scene{};
		std::vector<std::unique_ptr<shape::Mesh>> meshes{};
	};
}
