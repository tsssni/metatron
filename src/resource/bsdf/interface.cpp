#include <metatron/resource/bsdf/interface.hpp>
#include <metatron/core/math/constant.hpp>

namespace mtt::bsdf {
    auto Interface_Bsdf::operator()(
        math::Vector<f32, 3> const& wo,
        math::Vector<f32, 3> const& wi
    ) const noexcept -> std::optional<Interaction> {
        auto f = spectrum;
        f = 1.f;
        return Interaction{f, wo, 1.f};
    }

    auto Interface_Bsdf::sample(
        eval::Context const& ctx,
        math::Vector<f32, 3> const& u
    ) const noexcept -> std::optional<Interaction> {
        auto f = spectrum;
        f = 1.f;
        return Interaction{f, ctx.r.d, 1.f};
    }

    auto Interface_Bsdf::configure(Attribute const& attr) noexcept -> void {
        spectrum = attr.spectra.at("spectrum");
    }

    auto Interface_Bsdf::flags() const noexcept -> Flags {
        return Flags::interface;
    }

    auto Interface_Bsdf::degrade() noexcept -> bool {
        return false;
    }
}
