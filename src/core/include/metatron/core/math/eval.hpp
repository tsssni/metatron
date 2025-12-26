#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/math/transform.hpp>

namespace mtt::math {
    struct Context final {
        Ray r{};
        fv3 n{};
        fv4 lambda{};
        bool inside;
    };

    auto constexpr operator|(cref<Transform> t, cref<Context> ctx) -> Context {
        auto result = ctx;
        result.r = t | result.r;
        result.n = t | result.n;
        return result;
    }

    auto constexpr operator^(cref<Transform> t, cref<Context> ctx) -> Context {
        auto result = ctx;
        result.r = t ^ result.r;
        result.n = t ^ result.n;
        return result;
    }

    auto constexpr operator|(rref<Transform::Chain> chain, cref<Context> ctx) -> Context {
        auto result = ctx;
        result.r = chain | result.r;
        result.n = chain | result.n;
        return result;
    }

    auto constexpr operator^(rref<Transform::Chain> chain, cref<Context> ctx) -> Context {
        auto result = ctx;
        result.r = chain ^ result.r;
        result.n = chain ^ result.n;
        return result;
    }
}
