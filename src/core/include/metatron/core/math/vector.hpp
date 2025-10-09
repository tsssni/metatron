#pragma once
#include <metatron/core/math/matrix.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <cmath>

namespace mtt::math {
    template<typename T, usize size>
    using Vector = Matrix<T, size>;

    template<
        typename Func,
        typename... Ts,
        usize size
    >
    auto constexpr foreach(Func f, Vector<Ts, size> const&... vectors)
    noexcept -> Vector<decltype(f(vectors[0]..., 0uz)), size> {
        using Return_Type = decltype(f(vectors[0]..., 0uz));
        auto r = Vector<Return_Type, size>{};
        for (auto i = 0uz; i < size; i++) {
            r[i] = f(vectors[i]..., i);
        }
        return r;
    }

    template<
        typename Func,
        typename... Ts,
        usize size
    >
    auto constexpr any(Func f, Vector<Ts, size> const&... vectors) noexcept -> bool {
        using Return_Type = decltype(f(vectors[0]..., 0uz));
        static_assert(std::is_same_v<Return_Type, bool>, "f must return bool");

        auto r = foreach(f, vectors...);
        for (auto i = 0uz; i < size; i++) {
            if (r[i]) {
                return true;
            }
        }
        return false;
    }

    template<
        typename Func,
        typename... Ts,
        usize size
    >
    auto constexpr all(Func f, Vector<Ts, size> const&... vectors) noexcept -> bool {
        using Return_Type = decltype(f(vectors[0]..., 0uz));
        static_assert(std::is_same_v<Return_Type, bool>, "f must return bool");

        auto r = foreach(f, vectors...);
        for (auto i = 0uz; i < size; i++) {
            if (!r[i]) {
                return false;
            }
        }
        return true;
    }

    template<typename T, usize size>
    requires std::floating_point<T>
    auto constexpr isnan(Vector<T, size> const& x) noexcept -> bool {
        return math::any([](T x, auto){
            return math::isnan(x);
        }, x);
    }

    template<typename T, usize size>
    requires std::floating_point<T>
    auto constexpr isinf(Vector<T, size> const& x) noexcept -> bool {
        return math::any([](T x, auto){
            return math::isinf(x);
        }, x);
    }

    template<typename T, usize size>
    auto constexpr dot(Vector<T, size> const& x, Vector<T, size> const& y) noexcept -> T {
        auto result = T{};
        for (auto i = 0; i < size; i++) {
            result += x[i] * y[i];
        }
        return result;
    }

    template<typename T>
    auto constexpr cross(Vector<T, 3> const& x, Vector<T, 3> const& y) noexcept -> Vector<T, 3> {
        return {
            x[1] * y[2] - x[2] * y[1],
            x[2] * y[0] - x[0] * y[2],
            x[0] * y[1] - x[1] * y[0]
        };
    }

    template<typename T, typename U, typename V = decltype(T{} * U{}), usize size>
    auto constexpr mul(Vector<T, size> const& x, Vector<U, size> const& y) noexcept -> Vector<V, size> {
        return foreach([&](T const& v1, U const& v2, usize) noexcept -> V {
            return v1 * v2;
        }, x, y);
    }

    template<typename T, usize size>
    requires std::floating_point<T>
    auto constexpr length(Vector<T, size> const& x) noexcept -> T {
        return sqrt(dot(x, x));
    }

    template<typename T, usize size>
    requires std::floating_point<T>
    auto constexpr angle(Vector<T, size> const& x, Vector<T, size> const& y) noexcept -> T {
        // compute theta / 2 to avoid round-off error
        if (dot(x, y) < 0.f) {
            return pi - 2.f * std::asin(length(-y - x)/2);
        } else {
            return 2.f * std::asin(length(y - x) / 2);
        }
    }

    template<typename T, usize size>
    requires std::floating_point<T>
    auto constexpr normalize(Vector<T, size> const& x) noexcept -> Vector<T, size> {
        return x / length(x);
    }

    template<typename T>
    requires std::floating_point<T>
    auto constexpr reflect(Vector<T, 3> const& in, Vector<T, 3> const& n) noexcept -> Vector<T, 3> {
        return T{2.0} * n * dot(-in, n) + in; 
    }

    template<typename T>
    requires std::floating_point<T>
    auto constexpr refract(Vector<T, 3> const& in, Vector<T, 3> const& n, T const& eta) noexcept -> Vector<T, 3> {
        auto cos_theta_i = dot(-in, n);
        auto cos_2_theta_t = T{1.0} - (T{1.0} - sqr(cos_theta_i)) / sqr(eta);
        if (cos_2_theta_t < 0.0) return Vector<T, 3>{T{0.0}};
        return in / eta + (cos_theta_i / eta - sqrt(cos_2_theta_t)) * n;
    }

    template<typename T, typename... Ts, usize n, usize tail = sizeof...(Ts)>
    requires (std::is_convertible_v<T, Ts> && ...)
    auto constexpr expand(Vector<T, n> const& x, Ts... v) noexcept -> Vector<T, n + tail> {
        return Vector<T, n + sizeof...(v)>{x, v...};
    }

    template<typename T, typename... Ts, usize n, usize head = sizeof...(Ts)>
    requires (std::is_convertible_v<T, Ts> && ...)
    auto constexpr consume(Vector<T, n> const& x, Ts... v) noexcept -> Vector<T, n + head> {
        auto y = Vector<T, n + head>{};
        *(Vector<T, n>*)(y.data() + head) = x;
        *(Vector<T, head>*)(y.data()) = reverse(Vector<T, head>{v...});
        return y;
    }

    template<typename T, usize n, usize tail = 1uz>
    requires (n > tail)
    auto constexpr shrink(Vector<T, n> const& x) noexcept -> Vector<T, n - tail> {
        return Vector<T, n - tail>{x};
    }

    template<typename T, usize n, usize head = 1uz>
    requires (n > head)
    auto constexpr cut(Vector<T, n> const& x) noexcept -> Vector<T, n - head> {
        return *(view<Vector<T, n - head>>)(x.data() + head);
    }

    template<typename T, usize n>
    auto constexpr reverse(Vector<T, n> const& x) noexcept -> Vector<T, n> {
        return foreach([&](T const& v, usize i) noexcept -> T {
            return x[n - 1 - i];
        }, x);
    }

    template<typename T, usize size>
    requires std::totally_ordered<T>
    auto constexpr min(Vector<T, size> const& x) noexcept -> T {
        auto y = x[0];
        for (auto i = 1uz; i < size; i++) {
            y = std::min(y, x[i]);
        }
        return y;
    }

    template<typename... Ts, usize size>
    auto constexpr min(Vector<Ts, size> const&... xs) {
        return foreach([](Ts const&... xs, usize i) {
            return std::min({xs...});
        }, xs...);
    }

    template<typename T, usize size>
    auto constexpr mini(Vector<T, size> const& x) noexcept -> usize {
        auto const& x_arr = std::array<T, size>(x);
        return std::ranges::distance(x_arr.begin(), std::ranges::min_element(x_arr));
    }

    template<typename T, usize size>
    auto constexpr minvi(Vector<T, size> const& x) noexcept -> std::tuple<T, usize> {
        auto y = x[0];
        auto z = 0uz;
        for (auto i = 1uz; i < size; i++) {
            if (x[i] < y) {
                y = x[i];
                z = i;
            }
        }
        return std::make_tuple(y, z);
    }

    template<typename T, usize size>
    requires std::totally_ordered<T>
    auto constexpr max(Vector<T, size> const& x) noexcept -> T {
        auto y = x[0];
        for (auto i = 1uz; i < size; i++) {
            y = std::max(y, x[i]);
        }
        return y;
    }

    template<typename... Ts, usize size>
    auto constexpr max(Vector<Ts, size> const&... xs) {
        return foreach([](Ts const&... xs, usize i) {
            return std::max({xs...});
        }, xs...);
    }

    template<typename T, usize size>
    auto constexpr maxi(Vector<T, size> const& x) noexcept -> usize {
        auto const& x_arr = std::array<T, size>(x);
        return std::ranges::distance(x_arr.begin(), std::ranges::max_element(x_arr));
    }

    template<typename T, usize size>
    auto constexpr maxvi(Vector<T, size> const& x) noexcept -> std::tuple<T, usize> {
        auto y = x[0];
        auto z = 0uz;
        for (auto i = 1uz; i < size; i++) {
            if (x[i] > y) {
                y = x[i];
                z = i;
            }
        }
        return std::make_tuple(y, z);
    }

    template<typename T, usize size>
    requires std::floating_point<T> || std::integral<T>
    auto constexpr abs(Vector<T, size> const& x) noexcept -> Vector<T, size> {
        return foreach([](T const& v, usize) noexcept -> T {
            return math::abs(v);
        }, x);
    }

    template<typename T, usize size>
    requires std::floating_point<T>
    auto constexpr floor(Vector<T, size> const& x) noexcept -> Vector<T, size> {
        return foreach([](T const& v, usize) noexcept -> T {
            return std::floor(v);
        }, x);
    }

    template<typename T, usize size>
    requires std::floating_point<T>
    auto constexpr ceil(Vector<T, size> const& x) noexcept -> Vector<T, size> {
        return foreach([](T const& v, usize) noexcept -> T {
            return std::ceil(v);
        }, x);
    }

    template<typename T, usize size>
    requires std::floating_point<T>
    auto constexpr round(Vector<T, size> const& x) noexcept -> Vector<T, size> {
        return foreach([](T const& v, usize) noexcept -> T {
            return std::round(v);
        }, x);
    }

    template<typename T, usize size>
    requires requires(T a, T b) { a + b; }
    auto constexpr sum(Vector<T, size> const& x) noexcept -> T {
        auto y = T{};
        foreach([&y](T const& v, usize) noexcept -> T {
            y += v;
            return y;
        }, x);
        return y;
    }

    template<typename T, usize size>
    requires requires(T a, T b) { a * b; }
    auto constexpr prod(Vector<T, size> const& x) noexcept -> T {
        auto y = T{1};
        foreach([&y](T const& v, usize) noexcept -> T {
            y *= v;
            return y;
        }, x);
        return y;
    }

    template<typename T, usize size>
    requires(true
    && requires(T a, T b) { a + b; }
    && requires(T a) { a / 1uz; }
    )
    auto constexpr avg(Vector<T, size> const& x) noexcept -> T {
        return sum(x) / size;
    }

    template<typename T>
    requires std::floating_point<T> || std::integral<T>
    auto constexpr mod(T const& x, T const& m) noexcept -> T {
        if constexpr (std::floating_point<T>) {
            return std::fmod(x, m);
        } else {
            return x % m;
        };
    }

    template<typename T, usize size>
    requires std::floating_point<T> || std::integral<T>
    auto constexpr mod(Vector<T, size> const& x, T const& m) noexcept -> Vector<T, size> {
        return foreach([&](T const& v, usize i) noexcept -> T {
            if constexpr (std::floating_point<T>) {
                return std::fmod(v, m);
            } else {
                return v % m;
            }
        }, x);
    }

    template<typename T, usize size>
    requires std::floating_point<T> || std::integral<T>
    auto constexpr mod(Vector<T, size> const& x, Vector<T, size> const& m) noexcept -> Vector<T, size> {
        return foreach([&](T const& v, usize i) noexcept -> T {
            return math::mod(v, m[i]);
        }, x);
    }

    template<typename T, usize size>
    requires std::totally_ordered<T>
    auto constexpr clamp(Vector<T, size> const& x, Vector<T, size> const& l, Vector<T, size> const& r) noexcept -> Vector<T, size> {
        return foreach([&](T const& v, usize i) noexcept -> T {
            return math::clamp(v, l[i], r[i]);
        }, x);
    }

    template<typename T, usize size>
    requires std::floating_point<T>
    auto constexpr lerp(Vector<T, size> const& x, Vector<T, size> const& y, T const& alpha) noexcept -> Vector<T, size> {
        return (T{1.0} - alpha) * x + alpha * y;
    }

    template<typename T, usize size>
    requires std::floating_point<T>
    auto constexpr lerp(Vector<T, size> const& x, Vector<T, size> const& y, Vector<T, size> const& alpha) noexcept -> Vector<T, size> {
        return (T{1.0} - alpha) * x + alpha * y;
    }

    template<typename T, typename U, usize size>
    requires std::floating_point<U>
    auto constexpr blerp(Vector<T, size> const& x, Vector<U, size> const& b) noexcept -> T {
        return sum(mul(x, b));
    }

    template<typename T, usize size>
    requires std::floating_point<T>
    auto constexpr gram_schmidt(Vector<T, size> const& y, Vector<T, size> const& x) noexcept -> Vector<T, size> {
        return y - x * dot(x, y);
    }

    template<typename T, usize size>
    requires std::floating_point<T>
    auto constexpr orthogonalize(Vector<T, size> const& n) noexcept -> math::Matrix<T, 2, size> {
        auto t = math::abs(n[0]) > 1.f - math::epsilon<f32>
        ? math::Vector<f32, 3>{1.f, 0.f, 0.f}
        : math::Vector<f32, 3>{0.f, 1.f, 0.f};
        auto tn = math::normalize(math::gram_schmidt(t, n));
        auto bn = math::normalize(math::cross(tn, n));
        return {tn, bn};
    }
}
