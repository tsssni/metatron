#include <metatron/resource/bsdf/interface.hpp>

namespace mtt::bsdf {
    Interface_Bsdf::Interface_Bsdf(Descriptor const&) noexcept {}

    auto Interface_Bsdf::operator()(
        fv3 const& wo, fv3 const& wi, f32 u
    ) const noexcept -> Interaction {
        return {fv4{1.f}, fv4{1.f}, wo, 1.f};
    }

    auto Interface_Bsdf::sample(
        math::Context const& ctx, fv3 const& u
    ) const noexcept -> Interaction {
        return {fv4{1.f}, fv4{1.f}, ctx.r.d, 1.f};
    }

    auto Interface_Bsdf::flags() const noexcept -> Flags {
        return Flags::interface;
    }
}
