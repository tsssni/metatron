#pragma once
#include <metatron/core/math/matrix.hpp>
#include <print>
#include <format>

template<typename T, mtt::usize... dims>
struct std::formatter<mtt::math::Matrix<T, dims...>> {
    using M = mtt::math::Matrix<T, dims...>;

    auto constexpr parse(mtt::ref<std::format_parse_context> ctx) { return ctx.begin(); }

    auto format(
        mtt::cref<mtt::math::Matrix<T, dims...>> matrix,
        mtt::ref<std::format_context> ctx
    ) const {
        auto constexpr size = sizeof...(dims);
        auto out = std::format_to(ctx.out(), "[");
        auto constexpr first_dim = M::dimensions[0];
        for (auto i = 0uz; i < first_dim; ++i)
            out = i != 0 && size == 1
            ? std::format_to(out, ", {}", matrix[i])
            : std::format_to(out, "{}", matrix[i]);
        return std::format_to(ctx.out(), "]");
    }
};

namespace mtt::stl {
    template <typename... Args>
    auto print(std::format_string<Args...> format, Args&&... args) noexcept -> void {
        std::println(format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto abort(std::format_string<Args...> format, Args&&... args) noexcept -> void {
        std::println(format, std::forward<Args>(args)...);
        std::abort();
    }

    auto inline abort() noexcept { std::abort(); }
}
