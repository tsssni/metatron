#pragma once
#include <metatron/core/math/matrix.hpp>
#include <print>
#include <format>

template<typename T, mtt::usize... dims>
struct std::formatter<mtt::math::Matrix<T, dims...>> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }
    
    auto format(const mtt::math::Matrix<T, dims...>& matrix, std::format_context& ctx) const {
        if constexpr (sizeof...(dims) == 1) {
            auto out = std::format_to(ctx.out(), "[");
            constexpr auto size = mtt::math::Matrix<T, dims...>::dimensions[0];
            for (mtt::usize i = 0; i < size; ++i) {
                if (i > 0) out = std::format_to(out, ", ");
                out = std::format_to(out, "{}", matrix[i]);
            }
            return std::format_to(out, "]");
        } else {
            auto out = std::format_to(ctx.out(), "[");
            constexpr auto first_dim = mtt::math::Matrix<T, dims...>::dimensions[0];
            for (mtt::usize i = 0; i < first_dim; ++i) {
                if (i > 0) out = std::format_to(out, ",\n ");
                out = std::format_to(out, "{}", matrix[i]);
            }
            return std::format_to(out, "]");
        }
    }
};

