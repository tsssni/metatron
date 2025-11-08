#pragma once
#include <metatron/core/stl/array.hpp>

namespace mtt::stl {
    template<pro::facade F, typename... Ts>
    requires (sizeof...(Ts) > 0) && (poliable<F, Ts> && ...)
    struct variant final {
        using ts = stl::array<Ts...>;

        template<typename T, typename... Args>
        requires std::is_constructible_v<T, Args...> && ts::template contains<T>
        variant(Args&&... args) noexcept {emplace<T>(std::forward<Args>(args)...);};

        template<typename T>
        variant(T&& x) noexcept {push(std::forward<T>(x));};

        template<typename T, typename... Args>
        requires std::is_constructible_v<T, Args...> && ts::template contains<T>
        auto emplace(Args&&... args) noexcept -> void {
            idx = ts::template index<T>;
            std::construct_at(data<T>(), std::forward<Args>(args)...);
        }

        template<typename T>
        auto push(T&& x) noexcept -> void {
            emplace<T>(std::forward<T>(x));
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

        auto index() const noexcept -> u32 {
            return idx;
        }

        auto data() noexcept -> byte* {
            return storage.data();
        }

        template<typename T>
        auto data() noexcept -> T* {
            auto idx = ts::template index<T>;
            return (T*)data(idx);
        }

        auto size() const noexcept -> usize {
            return storage.size();
        }

    private:
        auto data(usize idx) const -> byte const* {
            auto offset = ((ts::template index<Ts> <= idx ? sizeof(Ts) : 0) + ...);
            return &storage[offset];
        }

        u32 idx;
        std::array<byte, (sizeof(Ts) + ...)> storage;
    };
}
