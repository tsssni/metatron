#pragma once
#include <metatron/render/material/interface.hpp>
#include <metatron/render/material/physical.hpp>
#include <metatron/core/stl/concept.hpp>

namespace mtt::render {

template<typename T>
concept Surfaced = requires(
    T const& t,
    core::Ray_Context const& ctx,
    fv2 const& uv, fv4 const& duvdxy
) {
    { t.sample(ctx, uv, duvdxy) } noexcept;
    requires Scattering<decltype(t.sample(ctx, uv, duvdxy).bsdf)>;
    { t.flags() } noexcept -> std::same_as<Material_Flags>;
};

using Material = core::protocol<Physical_Material, Interface_Material>;

}
