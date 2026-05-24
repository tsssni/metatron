#pragma once
#include <metatron/core/stl/print.hpp>

namespace mtt::stl {
    struct memory final {
        uptr bytes;
    };
}

template<>
struct std::formatter<mtt::stl::memory> {
    auto constexpr parse(mtt::ref<std::format_parse_context> ctx) { return ctx.begin(); }

    auto format(
        mtt::cref<mtt::stl::memory> mem,
        mtt::ref<std::format_context> ctx
    ) const {
        if (mem.bytes < (1uz << 10))
            return std::format_to(ctx.out(), "{} B", mem.bytes);
        else if (mem.bytes < (1uz << 20))
            return std::format_to(ctx.out(), "{:.2f} KB", mtt::f64(mem.bytes) / mtt::f64(1uz << 10));
        else if (mem.bytes < (1uz << 30))
            return std::format_to(ctx.out(), "{:.2f} MB", mtt::f64(mem.bytes) / mtt::f64(1uz << 20));
        else
            return std::format_to(ctx.out(), "{:.2f} GB", mtt::f64(mem.bytes) / mtt::f64(1uz << 30));
    }
};
