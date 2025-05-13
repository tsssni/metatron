#include <metatron/geometry/material/material.hpp>
#include <metatron/geometry/material/interface.hpp>

namespace metatron::material {
	std::unordered_set<std::type_index> Material::interface_materials;

	auto Material::initialize() -> void {
		interface_materials.insert(typeid(Interface_Material));
	}

	auto Material::is_interface(Material const& material) -> bool {
		return interface_materials.contains(typeid(material));
	}
}
