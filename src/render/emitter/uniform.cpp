#include <metatron/render/emitter/uniform.hpp>
#include <metatron/render/scene/hierarchy.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::emitter {
    Uniform_Emitter::Uniform_Emitter() {
        for (auto et: scene::entities<light::Light>()) {
            auto light = scene::fetch<light::Light>(et);
            auto t = scene::fetch<math::Transform>(et);
            prims.emplace_back(light, t);
            if (light->flags() & light::Flags::inf)
                inf_prims.emplace_back(light, t);
        }
    }

    auto Uniform_Emitter::sample(
        eval::Context const& ctx,
        f32 u
    ) const noexcept -> std::optional<Interaction> {
        if (prims.empty() && inf_prims.empty()) return {};
        auto idx = math::clamp(usize(u * prims.size()), 0uz, prims.size() - 1);
        auto prim = prims[idx];
        return Interaction{
            prim.light,
            prim.local_to_render,
            math::guarded_div(1.f, f32(prims.size() + inf_prims.size()))
        };
    }

    auto Uniform_Emitter::sample_infinite(
        eval::Context const& ctx,
        f32 u
    ) const noexcept -> std::optional<Interaction> {
        if (inf_prims.empty()) return {};
        auto idx = math::clamp(usize(u * prims.size()), 0uz, inf_prims.size() - 1);
        auto prim = inf_prims[idx];
        return Interaction{
            prim.light,
            prim.local_to_render,
            math::guarded_div(1.f, f32(inf_prims.size()))
        };
    }
}
