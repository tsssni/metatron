#pragma once
#include <metatron/resource/media/homogeneous.hpp>
#include <metatron/resource/media/heterogeneous.hpp>
#include <metatron/resource/media/vaccum.hpp>

namespace mtt::media {
    auto init() noexcept -> void;

    template<typename T>
    concept Marching = requires(T& t, f32 u) {
        { t.march(u) } noexcept;
        requires phase::Phase_Function<decltype(t.march(u).phase)>;
    };

    template<typename T>
    concept Participating = requires(
        T const& t,
        math::Context const& ctx,
        f32 t_max
    ) {
        { t.begin(ctx, t_max) } noexcept -> Marching;
    };

    template<typename T>
    struct Trait final: std::bool_constant<media::Participating<T>> {};
    using Medium = stl::seal<Trait, Homogeneous_Medium, Heterogeneous_Medium, Vaccum_Medium>;
}
