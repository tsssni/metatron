#pragma once
#include <memory>
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
        ~function() noexcept { if (table && table->destroyer) table->destroyer(*this); }

        template<typename E, typename F = std::decay_t<E>>
        requires (true
        && std::same_as<F, std::decay_t<E>>
        && !std::same_as<F, function>
        && std::is_nothrow_invocable_r_v<R, F&, Args...>)
        function(E&& f) noexcept {
            storage.resize(sizeof(F));
            std::construct_at((F*)storage.data(), std::forward<E>(f));
            table = &vtable<F>;
        }
        auto operator()(Args... args) const noexcept -> R { return table->caller(*this, std::forward<Args>(args)...); }
        operator bool() const noexcept { return table; }

        function(function&& rhs) noexcept { *this = std::move(rhs); }
        function(function const&) = delete;
        auto operator=(function const&) -> function& = delete;
        auto operator=(function&& rhs) noexcept -> function& {
            if (this == &rhs) return *this;
            if (table && table->destroyer) table->destroyer(*this);
            if (rhs.table && rhs.table->mover) rhs.table->mover(rhs, *this);
            else storage = std::move(rhs.storage);
            table = rhs.table;
            rhs.table = nullptr;
            return *this;
        }

    private:
        template<typename F>
        auto call(this function const& self, Args... args) noexcept -> R {
            return std::invoke(*(F*)self.storage.data(), std::forward<Args>(args)...);
        }

        template<typename F>
        auto move(this function& self, function& dst) noexcept -> void {
            dst.storage.resize(sizeof(F));
            std::construct_at((F*)dst.storage.data(), std::move(*(F*)self.storage.data()));
            std::destroy_at((F*)self.storage.data());
        }

        template<typename F>
        auto destroy(this function& self) noexcept -> void {
            std::destroy_at((F*)self.storage.data());
        }

        struct dispatch final {
            auto (*caller)(function const& self, Args... args) -> R = nullptr;
            auto (*mover)(function& self, function& dst) -> void = nullptr;
            auto (*destroyer)(function& self) -> void = nullptr;

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
        dispatch const* table = nullptr;
        std::string storage;
    };
}
