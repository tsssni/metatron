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
    auto constexpr foreach(Func f, cref<Vector<Ts, size>>... vectors)
    noexcept -> Vector<decltype(f(vectors[0]..., 0uz)), size> {
        using Return_Type = decltype(f(vectors[0]..., 0uz));
        auto r = Vector<Return_Type, size>{};
        for (auto i = 0uz; i < size; ++i)
            r[i] = f(vectors[i]..., i);
        return r;
    }

    template<
        typename Func,
        typename... Ts,
        usize size
    >
    auto constexpr any(Func f, cref<Vector<Ts, size>>... vectors) noexcept -> bool {
        using Return_Type = decltype(f(vectors[0]..., 0uz));
        static_assert(std::is_same_v<Return_Type, bool>, "f must return bool");

        auto r = foreach(f, vectors...);
        for (auto i = 0uz; i < size; ++i)
            if (r[i]) return true;
        return false;
    }

    template<
        typename Func,
        typename... Ts,
        usize size
    >
    auto constexpr all(Func f, cref<Vector<Ts, size>>... vectors) noexcept -> bool {
        using Return_Type = decltype(f(vectors[0]..., 0uz));
        static_assert(std::is_same_v<Return_Type, bool>, "f must return bool");

        auto r = foreach(f, vectors...);
        for (auto i = 0uz; i < size; ++i)
            if (!r[i]) return false;
        return true;
    }

    template<typename T, usize size>
    requires std::totally_ordered<T>
    auto constexpr constant(cref<Vector<T, size>> x) noexcept -> bool {
        return all([y = x[0]](T x, auto) {
            return x == y;
        }, x);
    }

    template<typename T, usize size>
    requires std::floating_point<T>
    auto constexpr isnan(cref<Vector<T, size>> x) noexcept -> bool {
        return any([](T x, auto){
            return isnan(x);
        }, x);
    }

    template<typename T, usize size>
    requires std::floating_point<T>
    auto constexpr isinf(cref<Vector<T, size>> x) noexcept -> bool {
        return any([](T x, auto){
            return isinf(x);
        }, x);
    }

    template<typename T, usize size>
    auto constexpr dot(cref<Vector<T, size>> x, cref<Vector<T, size>> y) noexcept -> T {
        auto result = T{};
        for (auto i = 0; i < size; ++i)
            result += x[i] * y[i];
        return result;
    }

    template<typename T>
    auto constexpr cross(cref<Vector<T, 3>> x, cref<Vector<T, 3>> y) noexcept -> Vector<T, 3> {
        return {
            x[1] * y[2] - x[2] * y[1],
            x[2] * y[0] - x[0] * y[2],
            x[0] * y[1] - x[1] * y[0]
        };
    }

    template<typename T, typename U, typename V = decltype(T{} * U{}), usize size>
    auto constexpr mul(cref<Vector<T, size>> x, cref<Vector<U, size>> y) noexcept -> Vector<V, size> {
        return foreach([&](cref<T> v1, cref<U> v2, usize) noexcept -> V {
            return v1 * v2;
        }, x, y);
    }

    template<typename T, usize size>
    requires std::floating_point<T>
    auto constexpr length(cref<Vector<T, size>> x) noexcept -> T {
        return sqrt(dot(x, x));
    }

    template<typename T, usize size>
    requires std::floating_point<T>
    auto constexpr angle(cref<Vector<T, size>> x, cref<Vector<T, size>> y) noexcept -> T {
        // compute theta / 2 to avoid round-off error
        if (dot(x, y) < 0.f) return pi - 2.f * std::asin(length(-y - x)/2);
        else return 2.f * std::asin(length(y - x) / 2);
    }

    template<typename T, usize size>
    requires std::floating_point<T>
    auto constexpr normalize(cref<Vector<T, size>> x) noexcept -> Vector<T, size> {
        return x / length(x);
    }

    template<typename T>
    requires std::floating_point<T>
    auto constexpr reflect(cref<Vector<T, 3>> in, cref<Vector<T, 3>> n) noexcept -> Vector<T, 3> {
        return T{2.0} * n * dot(-in, n) + in; 
    }

    template<typename T>
    requires std::floating_point<T>
    auto constexpr refract(cref<Vector<T, 3>> in, cref<Vector<T, 3>> n, cref<T> eta) noexcept -> Vector<T, 3> {
        auto cos_theta_i = dot(-in, n);
        auto cos_2_theta_t = T{1.0} - (T{1.0} - sqr(cos_theta_i)) / sqr(eta);
        if (cos_2_theta_t < 0.0) return Vector<T, 3>{T{0.0}};
        return in / eta + (cos_theta_i / eta - sqrt(cos_2_theta_t)) * n;
    }

    template<typename T, typename... Ts, usize n, usize tail = sizeof...(Ts)>
    requires (std::is_convertible_v<T, Ts> && ...)
    auto constexpr expand(cref<Vector<T, n>> x, Ts... v) noexcept -> Vector<T, n + tail> {
        return Vector<T, n + sizeof...(v)>{x, v...};
    }

    template<typename T, typename... Ts, usize n, usize head = sizeof...(Ts)>
    requires (std::is_convertible_v<T, Ts> && ...)
    auto constexpr consume(cref<Vector<T, n>> x, Ts... v) noexcept -> Vector<T, n + head> {
        auto y = Vector<T, n + head>{};
        *(Vector<T, n>*)(y.data() + head) = x;
        *(Vector<T, head>*)(y.data()) = reverse(Vector<T, head>{v...});
        return y;
    }

    template<typename T, usize n, usize tail = 1uz>
    requires (n > tail)
    auto constexpr shrink(cref<Vector<T, n>> x) noexcept -> Vector<T, n - tail> {
        return Vector<T, n - tail>{x};
    }

    template<typename T, usize n, usize head = 1uz>
    requires (n > head)
    auto constexpr cut(cref<Vector<T, n>> x) noexcept -> Vector<T, n - head> {
        return *(view<Vector<T, n - head>>)(x.data() + head);
    }

    template<typename T, usize n>
    auto constexpr reverse(cref<Vector<T, n>> x) noexcept -> Vector<T, n> {
        return foreach([&](cref<T> v, usize i) noexcept -> T {
            return x[n - 1 - i];
        }, x);
    }

    template<typename T, usize size>
    requires std::totally_ordered<T>
    auto constexpr min(cref<Vector<T, size>> x) noexcept -> T {
        return [&x]<usize... idxs>(std::index_sequence<idxs...>) -> T {
            return min(x[idxs]...);
        }(std::make_index_sequence<size>{});
    }

    template<typename... Ts, usize size>
    auto constexpr min(cref<Vector<Ts, size>>... xs) requires(sizeof...(xs) > 1) {
        return foreach([](cref<Ts>... xs, usize i) {
            return min(xs...);
        }, xs...);
    }

    template<typename T, usize size>
    auto constexpr mini(cref<Vector<T, size>> x) noexcept -> usize {
        auto const& x_arr = std::array<T, size>(x);
        return std::ranges::distance(x_arr.begin(), std::ranges::min_element(x_arr));
    }

    template<typename T, usize size>
    auto constexpr minvi(cref<Vector<T, size>> x) noexcept -> std::tuple<T, usize> {
        auto y = x[0];
        auto z = 0uz;
        for (auto i = 1uz; i < size; ++i)
            if (x[i] < y) {
                y = x[i];
                z = i;
            }
        return std::make_tuple(y, z);
    }

    template<typename T, usize size>
    requires std::totally_ordered<T>
    auto constexpr max(cref<Vector<T, size>> x) noexcept -> T {
        return [&x]<usize... idxs>(std::index_sequence<idxs...>) -> T {
            return max(x[idxs]...);
        }(std::make_index_sequence<size>{});
    }

    template<typename... Ts, usize size>
    auto constexpr max(cref<Vector<Ts, size>>... xs) requires(sizeof...(xs) > 1) {
        return foreach([](cref<Ts>... xs, usize i) {
            return math::max(xs...);
        }, xs...);
    }

    template<typename T, usize size>
    auto constexpr maxi(cref<Vector<T, size>> x) noexcept -> usize {
        auto const& x_arr = std::array<T, size>(x);
        return std::ranges::distance(x_arr.begin(), std::ranges::max_element(x_arr));
    }

    template<typename T, usize size>
    auto constexpr maxvi(cref<Vector<T, size>> x) noexcept -> std::tuple<T, usize> {
        auto y = x[0];
        auto z = 0uz;
        for (auto i = 1uz; i < size; ++i)
            if (x[i] > y) {
                y = x[i];
                z = i;
            }
        return std::make_tuple(y, z);
    }

    template<typename T, usize size>
    requires std::floating_point<T> || std::integral<T>
    auto constexpr abs(cref<Vector<T, size>> x) noexcept -> Vector<T, size> {
        return foreach([](cref<T> v, usize) noexcept -> T {
            return abs(v);
        }, x);
    }

    template<typename T, usize size>
    requires std::floating_point<T>
    auto constexpr floor(cref<Vector<T, size>> x) noexcept -> Vector<T, size> {
        return foreach([](cref<T> v, usize) noexcept -> T {
            return std::floor(v);
        }, x);
    }

    template<typename T, usize size>
    requires std::floating_point<T>
    auto constexpr ceil(cref<Vector<T, size>> x) noexcept -> Vector<T, size> {
        return foreach([](cref<T> v, usize) noexcept -> T {
            return std::ceil(v);
        }, x);
    }

    template<typename T, usize size>
    requires std::floating_point<T>
    auto constexpr round(cref<Vector<T, size>> x) noexcept -> Vector<T, size> {
        return foreach([](cref<T> v, usize) noexcept -> T {
            return std::round(v);
        }, x);
    }

    template<typename T, usize size>
    requires requires(T a, T b) { a + b; }
    auto constexpr sum(cref<Vector<T, size>> x) noexcept -> T {
        auto y = T{};
        foreach([&y](cref<T> v, usize) noexcept -> T {
            y += v;
            return y;
        }, x);
        return y;
    }

    template<typename T, usize size>
    requires requires(T a, T b) { a * b; }
    auto constexpr prod(cref<Vector<T, size>> x) noexcept -> T {
        auto y = T{1};
        foreach([&y](cref<T> v, usize) noexcept -> T {
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
    auto constexpr avg(cref<Vector<T, size>> x) noexcept -> T {
        return sum(x) / size;
    }

    template<typename T, usize size>
    requires std::floating_point<T> || std::integral<T>
    auto constexpr mod(cref<Vector<T, size>> x, cref<T> m) noexcept -> Vector<T, size> {
        return foreach([&](cref<T> v, usize i) noexcept -> T {
            return mod(v, m);
        }, x);
    }

    template<typename T, usize size>
    requires std::floating_point<T> || std::integral<T>
    auto constexpr mod(cref<Vector<T, size>> x, cref<Vector<T, size>> m) noexcept -> Vector<T, size> {
        return foreach([&](cref<T> v, usize i) noexcept -> T {
            return mod(v, m[i]);
        }, x);
    }

    template<typename T, usize size>
    requires std::totally_ordered<T>
    auto constexpr clamp(cref<Vector<T, size>> x, cref<Vector<T, size>> l, cref<Vector<T, size>> r) noexcept -> Vector<T, size> {
        return foreach([&](cref<T> v, usize i) noexcept -> T {
            return clamp(v, l[i], r[i]);
        }, x);
    }

    template<typename T, usize size>
    requires std::floating_point<T>
    auto constexpr lerp(cref<Vector<T, size>> x, cref<Vector<T, size>> y, cref<T> alpha) noexcept -> Vector<T, size> {
        return (T{1.0} - alpha) * x + alpha * y;
    }

    template<typename T, usize size>
    requires std::floating_point<T>
    auto constexpr lerp(cref<Vector<T, size>> x, cref<Vector<T, size>> y, cref<Vector<T, size>> alpha) noexcept -> Vector<T, size> {
        return (T{1.0} - alpha) * x + alpha * y;
    }

    template<typename T, typename U, usize size>
    requires std::floating_point<U>
    auto constexpr blerp(cref<Vector<T, size>> x, cref<Vector<U, size>> b) noexcept -> T {
        return sum(mul(x, b));
    }

    template<typename T, usize size>
    requires std::floating_point<T>
    auto constexpr gram_schmidt(cref<Vector<T, size>> y, cref<Vector<T, size>> x) noexcept -> Vector<T, size> {
        return y - x * dot(x, y);
    }

    template<typename T, usize size>
    requires std::floating_point<T>
    auto constexpr orthogonalize(cref<Vector<T, size>> n) noexcept -> Matrix<T, 2, size> {
        auto t = abs(n[0]) > 1.f - epsilon<f32>
        ? Vector<f32, 3>{1.f, 0.f, 0.f}
        : Vector<f32, 3>{0.f, 1.f, 0.f};
        auto tn = normalize(gram_schmidt(t, n));
        auto bn = normalize(cross(tn, n));
        return {tn, bn};
    }
}

namespace mtt {
    #define MTT_VECTOR_ALIAS(p, T)\
    template<usize n>\
    using p##v = math::Vector<T, n>;\
    \
    using p##v1 = p##v<1>;\
    using p##v2 = p##v<2>;\
    using p##v3 = p##v<3>;\
    using p##v4 = p##v<4>;

    MTT_VECTOR_ALIAS(f, f32)
    MTT_VECTOR_ALIAS(d, f64)
    MTT_VECTOR_ALIAS(i, i32)
    MTT_VECTOR_ALIAS(u, u32)
    MTT_VECTOR_ALIAS(ill, i64)
    MTT_VECTOR_ALIAS(ull, u64)
    MTT_VECTOR_ALIAS(b, byte)
    MTT_VECTOR_ALIAS(uz, usize)
}
