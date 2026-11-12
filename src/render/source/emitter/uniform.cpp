#include <metatron/render/emitter/uniform.hpp>

namespace mtt::render {

Uniform_Emitter::Uniform_Emitter(std::span<Primitive const> primitives) noexcept {
    auto prims = std::vector<Primitive>{};
    auto inf_prims = std::vector<Primitive>{};
    for (auto const& primitive : primitives) {
        prims.push_back(primitive);
        if (primitive.light.flags() & Light_Flags::inf) inf_prims.push_back(primitive);
    }

    num_prims = prims.size();
    num_inf_prims = inf_prims.size();
    this->prims = buf<Primitive>{num_prims};
    this->inf_prims = buf<Primitive>{num_inf_prims};
    this->prims.map();
    this->inf_prims.map();
    std::ranges::copy(prims, this->prims.ptr);
    std::ranges::copy(inf_prims, this->inf_prims.ptr);
    this->prims.unmap();
    this->inf_prims.unmap();
}

auto Uniform_Emitter::sample(
    core::Ray_Context const& ctx, f32 u
) const noexcept -> Emitter_Interaction {
    if (num_prims == 0 && num_inf_prims == 0) return {};
    auto idx = core::clamp(usize(u * num_prims), 0uz, num_prims - 1);
    auto prim = prims[idx];
    return {
        prim.light,
        prim.local_to_render,
        1.f / f32(num_prims)
    };
}

auto Uniform_Emitter::sample_infinite(
    core::Ray_Context const& ctx, f32 u
) const noexcept -> Emitter_Interaction {
    if (num_inf_prims == 0) return {};
    auto idx = core::clamp(usize(u * num_inf_prims), 0uz, num_inf_prims - 1);
    auto prim = inf_prims[idx];
    return {
        prim.light,
        prim.local_to_render,
        1.f / f32(num_inf_prims)
    };
}

}
