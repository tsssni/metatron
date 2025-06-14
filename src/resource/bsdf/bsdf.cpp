#include <metatron/resource/bsdf/interface.hpp>

namespace metatron::bsdf {
	auto Bsdf::is_interface(Bsdf const* bsdf) -> bool {
		return dynamic_cast<Interface_Bsdf const*>(bsdf) != nullptr;
	}
}
