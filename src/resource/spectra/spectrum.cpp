#include <metatron/resource/spectra/spectrum.hpp>

namespace mtt::spectra {
    std::unordered_map<std::string, proxy<Spectrum>> Spectrum::spectra;
}
