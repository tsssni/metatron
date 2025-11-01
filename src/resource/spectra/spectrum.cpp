#include <metatron/resource/spectra/spectrum.hpp>

namespace mtt::spectra {
    std::unordered_map<std::string, stl::proxy<Spectrum>> Spectrum::spectra;
}
