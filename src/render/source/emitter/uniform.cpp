#include <metatron/render/emitter/uniform.hpp>
#include <metatron/render/scene/hierarchy.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::emitter {
    Uniform_Emitter::Uniform_Emitter() {
        auto& lvec = stl::vector<light::Light>::instance();
        auto prims = std::vector<Primitive>{};
        auto inf_prims = std::vector<Primitive>{};
        for (auto const& et: lvec.keys()) {
            auto light = entity<light::Light>(et);
            auto t = entity<math::Transform>(et);
            prims.emplace_back(light, t);
            if (light->flags() & light::Flags::inf)
                inf_prims.emplace_back(light, t);
        }

        this->prims = std::span{prims};
        this->inf_prims = std::span{inf_prims};
    }

    auto Uniform_Emitter::sample(
        cref<math::Context> ctx, f32 u
    ) const noexcept -> opt<Interaction> {
        if (prims.empty() && inf_prims.empty()) return {};
        auto idx = math::clamp(usize(u * prims.size()), 0uz, prims.size() - 1);
        auto prim = prims[idx];
        return Interaction{
            prim.light,
            prim.local_to_render,
            math::guarded_div(1.f, f32(prims.size()))
        };
    }

    auto Uniform_Emitter::sample_infinite(
        cref<math::Context> ctx, f32 u
    ) const noexcept -> opt<Interaction> {
        if (inf_prims.empty()) return {};
        auto idx = math::clamp(usize(u * inf_prims.size()), 0uz, inf_prims.size() - 1);
        auto prim = inf_prims[idx];
        return Interaction{
            prim.light,
            prim.local_to_render,
            math::guarded_div(1.f, f32(inf_prims.size()))
        };
    }
}
