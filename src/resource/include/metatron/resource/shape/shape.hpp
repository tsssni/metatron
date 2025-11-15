#pragma once
#include <metatron/core/math/ray.hpp>
#include <metatron/core/math/bounding-box.hpp>
#include <metatron/resource/eval/context.hpp>

namespace mtt::shape {
    struct Interaction final {
        fv3 p;
        fv3 n;
        fv3 tn;
        fv3 bn;
        fv2 uv;
        f32 t;
        f32 pdf;

        fv3 dpdu;
        fv3 dpdv;
        fv3 dndu;
        fv3 dndv;
    };

    MTT_POLY_METHOD(shape_size, size);
    MTT_POLY_METHOD(shape_bounding_box, bounding_box);
    MTT_POLY_METHOD(shape_sample, sample);
    MTT_POLY_METHOD(shape_query, query);

    struct Shape final: pro::facade_builder
    ::add_convention<shape_size, auto () const noexcept -> usize>
    ::add_convention<shape_bounding_box, auto (
        cref<fm44> t, usize idx
    ) const noexcept -> math::Bounding_Box>
    ::add_convention<pro::operator_dispatch<"()">, auto (
        cref<math::Ray> r, cref<fv3> np, usize idx
    ) const noexcept -> opt<Interaction>>
    ::add_convention<shape_sample, auto (
        cref<eval::Context> ctx, cref<fv2> u, usize idx
    ) const noexcept -> opt<Interaction>>
    ::add_convention<shape_query, auto (
        cref<math::Ray> r, usize idx
    ) const noexcept -> opt<f32>>
    ::add_skill<pro::skills::as_view>
    ::build {};
}
