#pragma once
#include <cpptrace/cpptrace.hpp>
#include <format>
#include <print>

template<typename T, mtt::usize... dims>
struct std::formatter<mtt::core::matrix<T, dims...>> {
    using M = mtt::core::matrix<T, dims...>;
    auto constexpr parse(std::format_parse_context& ctx);
    auto format(
        mtt::core::matrix<T, dims...> const& matrix,
        std::format_context& ctx
    ) const;
};

template<typename T, mtt::usize... dims>
auto constexpr std::formatter<mtt::core::matrix<T, dims...>>::parse(std::format_parse_context& ctx) { return ctx.begin(); }

template<typename T, mtt::usize... dims>
auto std::formatter<mtt::core::matrix<T, dims...>>::format(
    mtt::core::matrix<T, dims...> const& matrix,
    std::format_context& ctx
) const {
    auto constexpr size = sizeof...(dims);
    auto out = std::format_to(ctx.out(), "[");
    auto constexpr dim0 = M::dimensions[0];
    for (auto i = 0uz; i < dim0; ++i)
        out = i != 0 && size == 1
            ? std::format_to(out, ", {}", matrix[i])
            : std::format_to(out, "{}", matrix[i]);
    return std::format_to(ctx.out(), "]");
}

namespace mtt::core {

template<typename... Args>
auto print(std::format_string<Args...> format, Args&&... args) noexcept -> void {
    std::println(format, std::forward<Args>(args)...);
}

template<typename... Args>
auto abort(std::format_string<Args...> format, Args&&... args) noexcept -> void {
    std::println(format, std::forward<Args>(args)...);
    cpptrace::generate_trace().print();
    std::fflush(stdout);
    std::fflush(stderr);
    std::abort();
}

auto inline abort() noexcept {
    cpptrace::generate_trace().print();
    std::fflush(stdout);
    std::fflush(stderr);
    std::abort();
}

}
