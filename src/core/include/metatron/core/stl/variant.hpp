#pragma once
#include <metatron/core/stl/array.hpp>
#include <metatron/core/math/arithmetic.hpp>

namespace mtt::stl {
    template<pro::facade F, typename... Ts>
    requires (sizeof...(Ts) > 0) && (poliable<F, Ts> && ...)
    struct variant final {
        using ts = stl::array<Ts...>;
        variant() noexcept = default;
        variant(variant&&) noexcept = default;
        ~variant() noexcept { destroy(); }

        template<typename T, typename... Args>
        requires std::is_constructible_v<T, Args...> && ts::template contains<T>
        variant(Args&&... args) noexcept {emplace<T>(std::forward<Args>(args)...);};

        template<typename T>
        requires ts::template contains<std::decay_t<T>>
        variant(T&& x) noexcept {push<std::decay_t<T>>(std::forward<T>(x));};

        template<typename T, typename... Args>
        requires std::is_constructible_v<T, Args...> && ts::template contains<T>
        auto emplace(Args&&... args) noexcept -> void {
            destroy();
            idx = ts::template index<T>;
            std::construct_at(data<T>(), std::forward<Args>(args)...);
        }

        template<typename T>
        requires ts::template contains<std::decay_t<T>>
        auto push(T&& x) noexcept -> void {
            emplace<std::decay_t<T>>(std::forward<T>(x));
        }

        auto operator->() noexcept -> mut<F> {
            auto ref = mut<F>{};
            ((
              idx == ts::template index<Ts>
              ? (ref = *(Ts*)data(idx), true) : false
            ) || ...);
            return ref;
        }

        auto operator->() const noexcept -> view<F> {
            return const_cast<variant*>(this)->operator->();
        }

        template<typename T>
        auto data() noexcept -> T* {
            auto idx = ts::template index<T>;
            return (T*)data(idx);
        }

        auto index() const noexcept -> u32 {return idx;}
        auto data() noexcept -> byte* {return storage.data();}
        auto size() const noexcept -> usize {return storage.size();}

    private:
        auto destroy() noexcept -> void {
            if (idx != math::maxv<u32>) auto v = ((
              idx == ts::template index<Ts>
              ? (std::destroy_at(data<Ts>()), true) : false
            ) || ...);
        }

        auto data(usize idx) const noexcept -> byte const* {
            auto offset = ((ts::template index<Ts> < idx ? sizeof(Ts) : 0) + ...);
            return &storage[offset];
        }

        u32 idx = math::maxv<u32>;
        std::array<byte, (sizeof(Ts) + ...)> storage;
    };
}
