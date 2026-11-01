#pragma once
#include <metatron/core/stl/array.hpp>

namespace mtt::core {

template<typename T>
concept polymorphic = requires { typename T::polymorphic_marker; };

template<typename... Ts>
struct protocol {
    using polymorphic_marker = u64;
    using ts = array<Ts...>;

    protocol() noexcept = default;
    explicit protocol(u64 value) noexcept;
    template<typename T>
    protocol(T* pointer) noexcept requires ts::template contains<T>;

    auto type() const noexcept -> u32;
    template<typename T>
    auto data() noexcept -> T*;
    template<typename T>
    auto data() const noexcept -> T const*;
    template<typename T>
    auto is() const noexcept -> bool;

    explicit operator u64() const noexcept;
    operator bool() const noexcept;

    template<typename S, typename Func>
    auto constexpr visit(this S&& self, Func&& f) -> decltype(auto);

private:
    auto static constexpr address = (u64{1} << 60) - 1;
    u64 value = 0;
};

template<typename... Ts>
protocol<Ts...>::protocol(u64 value) noexcept
: value(value) {}

template<typename... Ts>
template<typename T>
protocol<Ts...>::protocol(T* pointer) noexcept requires ts::template contains<T>
: value(((u64)ts::template index<T> << 60) | (u64)pointer) {}

template<typename... Ts>
auto protocol<Ts...>::type() const noexcept -> u32 { return (u32)(value >> 60); }

template<typename... Ts>
template<typename T>
auto protocol<Ts...>::data() noexcept -> T* { return (T*)(value & address); }

template<typename... Ts>
template<typename T>
auto protocol<Ts...>::data() const noexcept -> T const* { return (T const*)(value & address); }

template<typename... Ts>
template<typename T>
auto protocol<Ts...>::is() const noexcept -> bool { return type() == ts::template index<T>; }

template<typename... Ts>
protocol<Ts...>::operator u64() const noexcept { return value; }

template<typename... Ts>
protocol<Ts...>::operator bool() const noexcept { return (value & address) != 0; }

template<typename... Ts>
template<typename S, typename Func>
auto constexpr protocol<Ts...>::visit(this S&& self, Func&& f) -> decltype(auto) {
    using R = decltype(f(self.template data<typename ts::template type<0>>()));
    using SS = std::remove_reference_t<S>;
    return [&]<usize... Is>(std::index_sequence<Is...>) -> R {
        auto constexpr table = std::to_array({+[](SS& s, Func& f) -> R {
            return f(s.template data<typename ts::template type<Is>>());
        }...});
        return table[self.type()](self, f);
    }(std::make_index_sequence<sizeof...(Ts)>{});
}

}
