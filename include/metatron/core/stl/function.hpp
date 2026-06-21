#pragma once
#include <metatron/core/prelude/life.hpp>
#include <functional>
#include <type_traits>
#include <concepts>
#include <string>

namespace mtt::stl {
    template<typename Signature>
    struct function;

    template<typename R, typename... Args>
    struct function<R(Args...)noexcept> final {
        function() noexcept = default;
        ~function() noexcept { if (table && table->destroyer) (this->*(table->destroyer))(); }

        template<typename E, typename F = std::decay_t<E>>
        requires (true
        && std::same_as<F, std::decay_t<E>>
        && !std::same_as<F, function>
        && std::is_nothrow_invocable_r_v<R, ref<F>, Args...>)
        function(E&& f) noexcept {
            storage.resize(sizeof(F));
            std::construct_at((F*)storage.data(), std::forward<E>(f));
            table = &vtable<F>;
        }
        auto operator()(Args... args) const noexcept -> R { return (this->*(table->caller))(std::forward<Args>(args)...); }
        operator bool() const noexcept { return table; }

        function(rref<function> rhs) noexcept { *this = std::move(rhs); }
        function(cref<function>) = delete;
        auto operator=(cref<function>) -> function& = delete;
        auto operator=(rref<function> rhs) noexcept -> ref<function> {
            if (this == &rhs) return *this;
            if (table && table->destroyer) (this->*(table->destroyer))();
            if (rhs.table && rhs.table->mover) (rhs.*(rhs.table->mover))(*this);
            else storage = std::move(rhs.storage);
            table = rhs.table;
            rhs.table = nullptr;
            return *this;
        }

    private:
        template<typename F>
        auto call(Args... args) const noexcept -> R {
            return std::invoke(*(F*)storage.data(), std::forward<Args>(args)...);
        }

        template<typename F>
        auto move(ref<function> dst) noexcept -> void {
            dst.storage.resize(sizeof(F));
            std::construct_at((F*)dst.storage.data(), std::move(*(F*)storage.data()));
            std::destroy_at((F*)storage.data());
        }

        template<typename F>
        auto destroy() noexcept -> void {
            std::destroy_at((F*)storage.data());
        }

        struct dispatch final {
            auto (function::*caller)(Args... args) const -> R = nullptr;
            auto (function::*mover)(ref<function> dst) -> void = nullptr;
            auto (function::*destroyer)() -> void = nullptr;

            template<typename F>
            consteval dispatch(std::type_identity<F>) noexcept {
                caller = &function::call<F>;
                if constexpr (!(std::is_trivially_move_constructible_v<F> && std::is_trivially_destructible_v<F>))
                    mover = &function::move<F>;
                if constexpr (!std::is_trivially_destructible_v<F>)
                    destroyer = &function::destroy<F>;
            }
        };

        template<typename F>
        static constexpr dispatch vtable{std::type_identity<F>{}};
        view<dispatch> table = nullptr;
        std::string storage;
    };
}
