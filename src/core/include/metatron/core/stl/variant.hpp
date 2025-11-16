#pragma once
#include <metatron/core/stl/array.hpp>
#include <metatron/core/math/arithmetic.hpp>

namespace mtt::stl {
    template<pro::facade F, typename... Ts>
    requires (sizeof...(Ts) > 0) && (poliable<F, Ts> && ...)
    struct variant final {
        using ts = stl::array<Ts...>;
        variant(cref<variant>) noexcept = delete;
        variant(rref<variant> rhs) noexcept {
            idx = rhs.idx;
            storage = std::move(rhs.storage);
            rhs.idx = math::maxv<u32>; // skip destroy
        };
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
            std::construct_at((T*)data(idx), std::forward<Args>(args)...);
        }

        template<typename T>
        requires ts::template contains<std::decay_t<T>>
        auto push(T&& x) noexcept -> void {
            emplace<std::decay_t<T>>(std::forward<T>(x));
        }

        auto data() noexcept -> mut<F> {
            auto ref = mut<F>{};
            auto v = ((
              idx == ts::template index<Ts>
              ? (ref = make_mut(*(Ts*)data(idx)), true) : false
            ) || ...);
            return ref;
        }

        auto data() const noexcept -> view<F> {
            return const_cast<variant*>(this)->data();
        }

        auto clone() const noexcept -> obj<F>
        requires(F::copyability != pro::constraint_level::none) {
            auto ref = obj<F>{};
            auto v = ((
              idx == ts::template index<Ts>
              ? (ref = make_obj<F, Ts>(*(Ts const*)data(idx)), true) : false
            ) || ...);
            return ref;
        }

        auto operator->() noexcept -> mut<F> {return data();}
        auto operator->() const noexcept -> mut<F> {return data();}
        auto operator*() const noexcept -> obj<F> {return clone();}
        auto index() const noexcept -> u32 {return idx;}
        auto size() const noexcept -> usize {return storage.size();}

    private:
        auto destroy() noexcept -> void {
            if (idx != math::maxv<u32>) auto v = ((
              idx == ts::template index<Ts>
              ? (std::destroy_at(data(idx)), true) : false
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
