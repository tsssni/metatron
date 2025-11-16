#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/math/transform.hpp>

namespace mtt::bsdf {
    struct Bsdf;
}

namespace mtt::phase {
    struct Phase_Function;
}

namespace mtt::eval {
    struct Context final {
        math::Ray r{};
        fv3 n{};
        fv4 lambda{};
        bool inside;
    };

    auto inline operator|(cref<math::Transform> t, cref<Context> ctx) -> Context {
        auto result = ctx;
        result.r = t | result.r;
        result.n = t | result.n;
        return result;
    }

    auto inline operator^(cref<math::Transform> t, cref<Context> ctx) -> Context {
        auto result = ctx;
        result.r = t ^ result.r;
        result.n = t ^ result.n;
        return result;
    }

    auto inline operator|(rref<math::Transform::Chain> chain, cref<Context> ctx) -> Context {
        auto result = ctx;
        result.r = chain | result.r;
        result.n = chain | result.n;
        return result;
    }

    auto inline operator^(rref<math::Transform::Chain> chain, cref<Context> ctx) -> Context {
        auto result = ctx;
        result.r = chain ^ result.r;
        result.n = chain ^ result.n;
        return result;
    }
}
