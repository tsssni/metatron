#pragma once
#include <metatron/render/media/heterogeneous.hpp>
#include <metatron/render/media/homogeneous.hpp>
#include <metatron/render/media/vaccum.hpp>

namespace mtt::render {

template<typename T>
concept Marching = requires(T& t, f32 u) {
    { t.march(u) } noexcept;
    requires render::Phase_Function<decltype(t.march(u).phase)>;
};

template<typename T>
concept Participating = requires(
    T const& t,
    core::Ray_Context const& ctx,
    f32 t_max
) {
    { t.begin(ctx, t_max) } noexcept -> Marching;
};

using Medium = core::protocol<Homogeneous_Medium, Heterogeneous_Medium, Vaccum_Medium>;

}
