#pragma once
#include <metatron/core/stl/array.hpp>
#include <metatron/core/math/arithmetic.hpp>

namespace mtt::stl {
    template<pro::facade F, typename... Ts>
    requires (sizeof...(Ts) > 0) && (poliable<F, Ts> && ...)
    struct alignas(stl::array<Ts...>::alignment) variant final {
        using ts = stl::array<Ts...>;
        variant(cref<variant>) noexcept = delete;
        variant(rref<variant> rhs) noexcept {
            idx = rhs.idx;
            if (idx != math::maxv<u32>) auto v = ((
            idx == ts::template index<Ts> ? (
                std::construct_at(mut<Ts>(data(idx)), std::move(*mut<Ts>(rhs.data(idx))))
            , true) : false) || ...);
            rhs.idx = math::maxv<u32>; // skip destroy
        };
        ~variant() noexcept { destroy(); }

        template<typename T, typename... Args>
        requires std::is_constructible_v<T, Args...> && ts::template contains<T>
        variant(Args&&... args) noexcept {emplace<T>(std::forward<Args>(args)...);}

        template<typename T>
        requires ts::template contains<std::decay_t<T>>
        variant(T&& x) noexcept {push(std::forward<T>(x));}

        auto operator=(cref<variant>) noexcept -> variant& = delete;
        auto operator=(rref<variant> rhs) noexcept -> variant& {
            destroy();
            std::construct_at(this, std::move(rhs));
            return *this;
        }

        template<typename T, typename... Args>
        requires std::is_constructible_v<T, Args...> && ts::template contains<T>
        auto emplace(Args&&... args) noexcept -> void {
            destroy();
            idx = ts::template index<T>;
            std::construct_at(mut<T>(data(idx)), std::forward<Args>(args)...);
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
              ? (ref = make_mut(*mut<Ts>(data(idx))), true) : false
            ) || ...);
            return ref;
        }

        auto data() const noexcept -> view<F> {
            return const_cast<mut<variant>>(this)->data();
        }

        auto clone() const noexcept -> obj<F>
        requires(F::copyability != pro::constraint_level::none) {
            auto ref = obj<F>{};
            auto v = ((
              idx == ts::template index<Ts>
              ? (ref = make_obj<F, Ts>(*(view<Ts>)data(idx)), true) : false
            ) || ...);
            return ref;
        }

        auto operator->() noexcept -> mut<F> {return data();}
        auto operator->() const noexcept -> mut<F> {return data();}
        auto operator*() const noexcept -> obj<F> {return clone();}

        template<typename T>
        auto is() const noexcept -> bool { return ts::template index<T> == idx; }
        template<typename T>
        auto get() noexcept -> mut<T> { return mut<T>(&storage[ts::template offset<T>]); }
        template<typename T>
        auto get() const noexcept -> view<T> { return view<T>(&storage[ts::template offset<T>]); }
        auto index() const noexcept -> u32 {return idx;}
        auto size() const noexcept -> usize {return storage.size();}

    private:
        auto destroy() noexcept -> void {
            if (idx != math::maxv<u32>) auto v = ((
            idx == ts::template index<Ts> ? (
                std::destroy_at(mut<Ts>(data(idx)))
            , true) : false) || ...);
        }

        auto data(usize idx) const noexcept -> view<byte> {
            auto offset = ((ts::template index<Ts> == idx ? ts::template offset<Ts> : 0) + ...);
            return &storage[offset];
        }

        template<typename T, typename U>
        auto test() {
            auto x = std::is_same<T, U>::type;
        }

        std::array<byte, ts::size> storage;
        u32 idx = math::maxv<u32>;
    };
}
