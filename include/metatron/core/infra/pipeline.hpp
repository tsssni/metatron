#pragma once
#include <metatron/core/stl/protocol.hpp>

namespace mtt::core {

struct cartesian final {
    template<typename F, polymorphic... Ts>
    auto operator()(F&& f, Ts... tags) const noexcept -> void;

private:
    template<typename... Us, typename F>
    auto visit(F& f) const noexcept -> void;
    template<typename... Us, typename F, typename T, typename... Ts>
    auto visit(F& f, T tag, Ts... tags) const noexcept -> void;
};

template<typename F, polymorphic... Ts>
auto cartesian::operator()(F&& f, Ts... tags) const noexcept -> void { visit(f, tags...); }

template<typename... Us, typename F>
auto cartesian::visit(F& f) const noexcept -> void {
    if constexpr (sizeof...(Us) == 0) f();
    else f.template operator()<Us...>();
}

template<typename... Us, typename F, typename T, typename... Ts>
auto cartesian::visit(F& f, T tag, Ts... tags) const noexcept -> void {
    [&]<typename... Vs>(std::type_identity<array<Vs...>>) {
        auto g = [&]<typename V>() {
            if (tag.type() != T::ts::template index<V>) return false;
            this->template visit<Us..., V>(f, tags...);
            return true;
        };
        (void)(g.template operator()<Vs>() || ...);
    }(std::type_identity<typename T::ts>{});
}

template<typename F, typename C>
struct pipenc final {
    pipenc(recorder& r, F& f, C& c) noexcept;
    ~pipenc() noexcept;

    auto dispatch(uv3 groups, uv3 invocs) noexcept -> void;
    auto dispatch(uv3 size) noexcept -> void;
};

}
