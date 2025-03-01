#include <metatron/core/math/vector.hpp>

namespace metatron::spectra {
	struct Spectrum {
		explicit virtual operator math::Vector<f32, 3>() const = 0;
	};
}
