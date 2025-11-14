#include <metatron/resource/bsdf/interface.hpp>
#include <metatron/core/math/constant.hpp>

namespace mtt::bsdf {
    Interface_Bsdf::Interface_Bsdf(
        cref<stsp> spectrum
    ) noexcept: spectrum(spectrum) {}

    auto Interface_Bsdf::operator()(
        cref<fv3> wo, cref<fv3> wi
    ) const noexcept -> opt<Interaction> {
        auto f = spectrum;
        f = 1.f;
        return Interaction{f, wo, 1.f};
    }

    auto Interface_Bsdf::sample(
        cref<eval::Context> ctx, cref<fv3> u
    ) const noexcept -> opt<Interaction> {
        auto f = spectrum;
        f = 1.f;
        return Interaction{f, ctx.r.d, 1.f};
    }

    auto Interface_Bsdf::flags() const noexcept -> Flags {
        return Flags::interface;
    }

    auto Interface_Bsdf::degrade() noexcept -> bool {
        return false;
    }
}
