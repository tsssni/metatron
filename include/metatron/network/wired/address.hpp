#pragma once
#include <metatron/core/stl/print.hpp>

namespace mtt::wired {
    struct Address final {
        std::string host{};
        std::string port{};
        Address() noexcept = default;
        Address(std::string_view address) noexcept;
        Address(cref<Address>) noexcept = default;
    };
}

template<>
struct std::formatter<mtt::wired::Address> {
    constexpr auto parse(mtt::ref<std::format_parse_context> ctx) {
        return ctx.begin();
    }

    auto format(
        mtt::cref<mtt::wired::Address> addr,
        mtt::ref<std::format_context> ctx
    ) const {
        return std::format_to(ctx.out(), "{}:{}", addr.host, addr.port);
    }
};
