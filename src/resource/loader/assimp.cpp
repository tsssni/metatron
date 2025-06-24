#include <metatron/resource/loader/assimp.hpp>
#include <metatron/resource/texture/constant.hpp>
#include <metatron/core/math/sphere.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace metatron::loader {
	auto Assimp_Loader::from_path(std::string_view path) -> std::vector<std::unique_ptr<shape::Mesh>> {
		auto importer = Assimp::Importer{};
		scene = importer.ReadFile(path.data(), 0
			| aiProcess_FindDegenerates
			| aiProcess_FlipUVs
			| aiProcess_FlipWindingOrder
			| aiProcess_GenSmoothNormals
			| aiProcess_GenUVCoords
			| aiProcess_ImproveCacheLocality
			| aiProcess_JoinIdenticalVertices
			| aiProcess_MakeLeftHanded
			| aiProcess_OptimizeGraph
			| aiProcess_OptimizeMeshes
			| aiProcess_RemoveRedundantMaterials
			| aiProcess_TransformUVCoords
			| aiProcess_Triangulate
		);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->HasMeshes()) {
			std::printf("assimp error: while loading %s: %s\n", path.data(), importer.GetErrorString());
			std::abort();
		}

		traverse(scene->mRootNode);
		return std::move(meshes);
	}


	auto Assimp_Loader::traverse(aiNode const* node) -> void {
		for (auto i = 0uz; i < node->mNumMeshes; i++) {
			auto mesh = scene->mMeshes[node->mMeshes[i]];
			load_mesh(mesh);
		}
		for (auto i = 0uz; i < node->mNumChildren; i++) {
			traverse(node->mChildren[i]);
		}
	}

	auto Assimp_Loader::load_mesh(aiMesh const* mesh) -> void {
		auto indices = std::vector<math::Vector<usize, 3>>{};
		auto vertices = std::vector<math::Vector<f32, 3>>{};
		auto normals = std::vector<math::Vector<f32, 3>>{};
		auto uvs = std::vector<math::Vector<f32, 2>>{};

		for (auto i = 0uz; i < mesh->mNumFaces; i++) {
			auto face = mesh->mFaces[i];
			if (face.mNumIndices != 3) {
				// std::printf("assimp error: mesh %s has non-triangle face\n", mesh->mName.C_Str());
				// std::abort();
				continue;
			}
			indices.push_back({
				face.mIndices[0],
				face.mIndices[1],
				face.mIndices[2]
			});
		}

		for (auto i = 0uz; i < mesh->mNumVertices; i++) {
			vertices.push_back({
				mesh->mVertices[i].x,
				mesh->mVertices[i].y,
				mesh->mVertices[i].z
			});
			normals.push_back({
				mesh->mNormals[i].x,
				mesh->mNormals[i].y,
				mesh->mNormals[i].z
			});
			uvs.push_back(mesh->mTextureCoords[0]
				? math::Vector<f32, 2>{
					mesh->mTextureCoords[0][i].x,
					mesh->mTextureCoords[0][i].y
				}
				: 1.f
				* math::cartesion_to_unit_sphere(math::normalize(vertices.back()))
				/ math::Vector<f32, 2>{math::pi, 2.f * math::pi}
			);
		}

		meshes.push_back(std::make_unique<shape::Mesh>(
			std::move(indices),
			std::move(vertices),
			std::move(normals),
			std::move(uvs)
		));
	}
}
