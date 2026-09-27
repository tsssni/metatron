#include <metatron/resource/phase/vaccum.hpp>

namespace mtt::phase {
    auto Vaccum_Phase_Function::operator()(
        fv3 const& wo, fv3 const& wi
    ) const noexcept -> Interaction { return {}; }

    auto Vaccum_Phase_Function::sample(
        math::Context const& ctx, fv2 const& u
    ) const noexcept -> Interaction { return {}; }
}
