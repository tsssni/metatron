#include <metatron/resource/bsdf/interface.hpp>
#include <metatron/core/math/constant.hpp>

namespace mtt::bsdf {
    Interface_Bsdf::Interface_Bsdf(cref<Descriptor>) noexcept {}

    auto Interface_Bsdf::operator()(
        cref<fv3> wo, cref<fv3> wi
    ) const noexcept -> opt<Interaction> {
        return Interaction{fv4{1.f}, wo, 1.f};
    }

    auto Interface_Bsdf::sample(
        cref<math::Context> ctx, cref<fv3> u
    ) const noexcept -> opt<Interaction> {
        return Interaction{fv4{1.f}, ctx.r.d, 1.f};
    }

    auto Interface_Bsdf::flags() const noexcept -> Flags {
        return Flags::interface;
    }
}
