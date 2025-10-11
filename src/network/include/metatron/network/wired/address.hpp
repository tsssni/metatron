#pragma once
#include <metatron/core/stl/print.hpp>

namespace mtt::wired {
    struct Address final {
        std::string host{};
        std::string port{};
        Address() noexcept = default;
        Address(std::string_view address) noexcept;
        Address(Address const&) noexcept = default;
    };
}

template<>
struct std::formatter<mtt::wired::Address> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(mtt::wired::Address const& addr, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{}:{}", addr.host, addr.port);
    }
};
