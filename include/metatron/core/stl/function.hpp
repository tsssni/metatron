#pragma once
#include <metatron/core/stl/concept.hpp>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>

namespace mtt::core {

template<typename Signature>
struct function;

template<typename R, typename... Args>
struct function<R(Args...) noexcept> final {
    function() noexcept = default;
    ~function() noexcept;

    template<typename E, typename F = std::decay_t<E>>
    function(E&& f) noexcept requires(std::same_as<F, std::decay_t<E>> && !std::same_as<F, function> && std::is_nothrow_invocable_r_v<R, F&, Args...>);
    auto operator()(Args... args) const noexcept -> R;
    operator bool() const noexcept;

    function(function&& rhs) noexcept;
    function(function const&) = delete;
    auto operator=(function const&) -> function& = delete;
    auto operator=(function&& rhs) noexcept -> function&;

private:
    template<typename F>
    auto call(this function const& self, Args... args) noexcept -> R;
    template<typename F>
    auto move(this function& self, function& dst) noexcept -> void;
    template<typename F>
    auto destroy(this function& self) noexcept -> void;

    struct dispatch final {
        auto (*caller)(function const& self, Args... args) -> R = nullptr;
        auto (*mover)(function& self, function& dst) -> void = nullptr;
        auto (*destroyer)(function& self) -> void = nullptr;

        template<typename F>
        consteval dispatch(std::type_identity<F>) noexcept;
    };

    template<typename F>
    static constexpr dispatch vtable{std::type_identity<F>{}};
    dispatch const* table = nullptr;
    std::string storage;
};

template<typename R, typename... Args>
function<R(Args...) noexcept>::~function() noexcept {
    if (table && table->destroyer) table->destroyer(*this);
}

template<typename R, typename... Args>
template<typename E, typename F>
function<R(Args...) noexcept>::function(E&& f) noexcept requires(std::same_as<F, std::decay_t<E>> && !std::same_as<F, function> && std::is_nothrow_invocable_r_v<R, F&, Args...>) {
    storage.resize(sizeof(F));
    std::construct_at((F*)storage.data(), std::forward<E>(f));
    table = &vtable<F>;
}

template<typename R, typename... Args>
auto function<R(Args...) noexcept>::operator()(Args... args) const noexcept -> R { return table->caller(*this, std::forward<Args>(args)...); }
template<typename R, typename... Args>
function<R(Args...) noexcept>::operator bool() const noexcept { return table; }
template<typename R, typename... Args>
function<R(Args...) noexcept>::function(function&& rhs) noexcept { *this = std::move(rhs); }

template<typename R, typename... Args>
auto function<R(Args...) noexcept>::operator=(function&& rhs) noexcept -> function& {
    if (this == &rhs) return *this;
    if (table && table->destroyer) table->destroyer(*this);
    if (rhs.table && rhs.table->mover) rhs.table->mover(rhs, *this);
    else storage = std::move(rhs.storage);
    table = rhs.table;
    rhs.table = nullptr;
    return *this;
}

template<typename R, typename... Args>
template<typename F>
auto function<R(Args...) noexcept>::call(this function const& self, Args... args) noexcept -> R {
    return std::invoke(*(F*)self.storage.data(), std::forward<Args>(args)...);
}

template<typename R, typename... Args>
template<typename F>
auto function<R(Args...) noexcept>::move(this function& self, function& dst) noexcept -> void {
    dst.storage.resize(sizeof(F));
    std::construct_at((F*)dst.storage.data(), std::move(*(F*)self.storage.data()));
    std::destroy_at((F*)self.storage.data());
}

template<typename R, typename... Args>
template<typename F>
auto function<R(Args...) noexcept>::destroy(this function& self) noexcept -> void {
    std::destroy_at((F*)self.storage.data());
}

template<typename R, typename... Args>
template<typename F>
consteval function<R(Args...) noexcept>::dispatch::dispatch(std::type_identity<F>) noexcept {
    caller = &function::call<F>;
    if constexpr (!(std::is_trivially_move_constructible_v<F> && std::is_trivially_destructible_v<F>))
        mover = &function::move<F>;
    if constexpr (!std::is_trivially_destructible_v<F>)
        destroyer = &function::destroy<F>;
}

}
