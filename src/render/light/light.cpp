#include <metatron/render/light/light.hpp>
#include <metatron/render/light/parallel.hpp>

namespace metatron::light {
	std::unordered_set<std::type_index> Light::delta_lights;
	
	auto Light::initialize() -> void {
		delta_lights.insert(typeid(Parallel_Light));
	}

	auto Light::is_delta(Light const& light) -> bool {
		return delta_lights.contains(typeid(light));
	}
}
