#pragma once
#include <metatron/core/infra/matrix.hpp>
#include <metatron/core/math/cmath.hpp>
#include <metatron/core/stl/concept.hpp>

namespace mtt::core {

template<typename T, usize size>
using vector = matrix<T, size>;

template<typename Func, typename... Ts, usize size>
auto constexpr foreach(Func f, vector<Ts, size> const&... vs) noexcept -> vector<decltype(f(vs[0]..., 0uz)), size> {
    using R = decltype(f(vs[0]..., 0uz));
    auto r = vector<R, size>{};
    for (auto i = 0uz; i < size; ++i)
        r[i] = f(vs[i]..., i);
    return r;
}

template<typename Func, typename... Ts, usize size>
auto constexpr any(Func f, vector<Ts, size> const&... vs) noexcept -> bool requires std::same_as<decltype(f(vs[0]..., 0uz)), bool> {
    auto r = foreach(f, vs...);
    for (auto i = 0uz; i < size; ++i)
        if (r[i]) return true;
    return false;
}

template<typename Func, typename... Ts, usize size>
auto constexpr all(Func f, vector<Ts, size> const&... vs) noexcept -> bool requires std::same_as<decltype(f(vs[0]..., 0uz)), bool> {
    auto r = foreach(f, vs...);
    for (auto i = 0uz; i < size; ++i)
        if (!r[i]) return false;
    return true;
}

template<std::totally_ordered T, usize size>
auto constexpr constant(vector<T, size> const& x) noexcept -> bool {
    return all([y = x[0]](T x, auto) { return x == y; }, x);
}

template<floating_point T, usize size>
auto constexpr isnan(vector<T, size> const& x) noexcept -> bool {
    return any([](T x, auto) { return isnan(x); }, x);
}

template<floating_point T, usize size>
auto constexpr isinf(vector<T, size> const& x) noexcept -> bool {
    return any([](T x, auto) { return isinf(x); }, x);
}

template<floating_point T, usize size>
auto constexpr isfinite(vector<T, size> const& x) noexcept -> bool {
    return all([](T x, auto) { return isfinite(x); }, x);
}

template<typename T, usize size>
auto constexpr dot(vector<T, size> const& x, vector<T, size> const& y) noexcept -> T {
    auto result = T{};
    for (auto i = 0; i < size; ++i)
        result += x[i] * y[i];
    return result;
}

template<typename T>
auto constexpr cross(vector<T, 3> const& x, vector<T, 3> const& y) noexcept -> vector<T, 3> {
    return {
        x[1] * y[2] - x[2] * y[1],
        x[2] * y[0] - x[0] * y[2],
        x[0] * y[1] - x[1] * y[0]
    };
}

template<typename T, typename U, typename V = decltype(T{} * U{}), usize size>
auto constexpr mul(vector<T, size> const& x, vector<U, size> const& y) noexcept -> vector<V, size> {
    return foreach([&](T const& v1, U const& v2, usize) noexcept -> V { return v1 * v2; }, x, y);
}

template<floating_point T, usize size>
auto constexpr length(vector<T, size> const& x) noexcept -> T { return sqrt(dot(x, x)); }

template<floating_point T, usize size>
auto constexpr angle(vector<T, size> const& x, vector<T, size> const& y) noexcept -> T {
    // compute theta / 2 to avoid round-off error
    if (dot(x, y) < 0.f) return pi - 2.f * asin(length(-y - x) / 2);
    else return 2.f * asin(length(y - x) / 2);
}

template<floating_point T, usize size>
auto constexpr normalize(vector<T, size> const& x) noexcept -> vector<T, size> {
    return x / length(x);
}

template<floating_point T>
auto constexpr reflect(vector<T, 3> const& in, vector<T, 3> const& n) noexcept -> vector<T, 3> {
    return T(2.0) * n * dot(-in, n) + in;
}

template<floating_point T>
auto constexpr refract(vector<T, 3> const& in, vector<T, 3> const& n, T const& eta) noexcept -> vector<T, 3> {
    auto cos_theta_i = dot(-in, n);
    auto cos_2_theta_t = T(1.0) - (T(1.0) - sqr(cos_theta_i)) / sqr(eta);
    if (cos_2_theta_t < 0.0) return vector<T, 3>{T(0.0)};
    return in / eta + (cos_theta_i / eta - sqrt(cos_2_theta_t)) * n;
}

template<typename T, typename... Ts, usize n, usize tail = sizeof...(Ts)>
auto constexpr expand(vector<T, n> const& x, Ts... v) noexcept -> vector<T, n + tail> requires(std::convertible_to<T, Ts> && ...) {
    return vector<T, n + sizeof...(v)>{x, v...};
}

template<typename T, typename... Ts, usize n, usize head = sizeof...(Ts)>
auto constexpr consume(vector<T, n> const& x, Ts... v) noexcept -> vector<T, n + head> requires(std::convertible_to<T, Ts> && ...) {
    return vector<T, n + head>{reverse(vector<T, head>{v...}), x};
}

template<typename T, usize n, usize tail = 1uz>
auto constexpr shrink(vector<T, n> const& x) noexcept -> vector<T, n - tail> requires(n > tail) {
    return vector<T, n - tail>{x};
}

template<typename T, usize n, usize head = 1uz>
auto constexpr cut(vector<T, n> const& x) noexcept -> vector<T, n - head> requires(n > head) {
    return *(vector<T, n - head> const*)(x.data() + head);
}

template<typename T, usize n>
auto constexpr reverse(vector<T, n> const& x) noexcept -> vector<T, n> {
    return foreach([&](T const& v, usize i) noexcept -> T { return x[n - 1 - i]; }, x);
}

template<std::totally_ordered T, usize size>
auto constexpr min(vector<T, size> const& x) noexcept -> T {
    return [&x]<usize... idxs>(std::index_sequence<idxs...>) -> T { return min(x[idxs]...); }(std::make_index_sequence<size>{});
}

template<typename... Ts, usize size>
auto constexpr min(vector<Ts, size> const&... xs) requires(sizeof...(xs) > 1) {
    return foreach([](Ts const&... xs, usize i) { return min(xs...); }, xs...);
}

template<typename T, usize size>
auto constexpr mini(vector<T, size> const& x) noexcept -> usize {
    auto const& x_arr = std::array<T, size>(x);
    return std::ranges::distance(x_arr.begin(), std::ranges::min_element(x_arr));
}

template<typename T, usize size>
auto constexpr minvi(vector<T, size> const& x) noexcept -> std::tuple<T, usize> {
    auto y = x[0];
    auto z = 0uz;
    for (auto i = 1uz; i < size; ++i)
        if (x[i] < y) {
            y = x[i];
            z = i;
        }
    return std::make_tuple(y, z);
}

template<std::totally_ordered T, usize size>
auto constexpr max(vector<T, size> const& x) noexcept -> T {
    return [&x]<usize... idxs>(std::index_sequence<idxs...>) -> T { return max(x[idxs]...); }(std::make_index_sequence<size>{});
}

template<typename... Ts, usize size>
auto constexpr max(vector<Ts, size> const&... xs) requires(sizeof...(xs) > 1) {
    return foreach([](Ts const&... xs, usize i) { return max(xs...); }, xs...);
}

template<typename T, usize size>
auto constexpr maxi(vector<T, size> const& x) noexcept -> usize {
    auto y = std::span<T const, size>(x);
    return std::ranges::distance(y.begin(), std::ranges::max_element(y));
}

template<typename T, usize size>
auto constexpr maxvi(vector<T, size> const& x) noexcept -> std::tuple<T, usize> {
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
auto constexpr abs(vector<T, size> const& x) noexcept -> vector<T, size> requires floating_point<T> || std::integral<T> {
    return foreach([](T const& v, usize) noexcept -> T { return abs(v); }, x);
}

template<floating_point T, usize size>
auto constexpr floor(vector<T, size> const& x) noexcept -> vector<T, size> {
    return foreach([](T const& v, usize) noexcept -> T { return floor(v); }, x);
}

template<floating_point T, usize size>
auto constexpr ceil(vector<T, size> const& x) noexcept -> vector<T, size> {
    return foreach([](T const& v, usize) noexcept -> T { return ceil(v); }, x);
}

template<floating_point T, usize size>
auto constexpr round(vector<T, size> const& x) noexcept -> vector<T, size> {
    return foreach([](T const& v, usize) noexcept -> T { return round(v); }, x);
}

template<typename T, usize size>
auto constexpr sum(vector<T, size> const& x) noexcept -> T requires requires(T a, T b) { a + b; } {
    auto y = T{};
    foreach([&y](T const& v, usize) noexcept -> T { y += v; return y; }, x);
    return y;
}

template<typename T, usize size>
auto constexpr prod(vector<T, size> const& x) noexcept -> T requires requires(T a, T b) { a * b; } {
    auto y = T(1);
    foreach([&y](T const& v, usize) noexcept -> T { y *= v; return y; }, x);
    return y;
}

template<typename T, usize size>
auto constexpr avg(vector<T, size> const& x) noexcept -> T requires(requires(T a, T b) { a + b; a / 1uz; }) { return sum(x) / size; }

template<floating_point T, usize size>
auto constexpr cbrt(vector<T, size> const& x) noexcept -> vector<T, size> {
    return foreach([](T const& v, auto) { return cbrt(v); }, x);
}

template<usize n, usize d = 1, typename T, usize size>
auto constexpr pow(vector<T, size> const& x) noexcept -> vector<T, size> requires(d == 1 || floating_point<T>) {
    return foreach([](T const& v, auto) { return pow<n, d>(v); }, x);
}

template<typename T, usize size>
auto constexpr exp(vector<T, size> const& x) noexcept -> vector<T, size> {
    return foreach([](T const& v, auto) { return exp(v); }, x);
}

template<typename T, usize size>
auto constexpr mod(vector<T, size> const& x, T const& m) noexcept -> vector<T, size> requires floating_point<T> || std::integral<T> {
    return foreach([&](T const& v, usize i) noexcept -> T { return mod(v, m); }, x);
}

template<typename T, usize size>
auto constexpr mod(vector<T, size> const& x, vector<T, size> const& m) noexcept -> vector<T, size> requires floating_point<T> || std::integral<T> {
    return foreach([&](T const& v, usize i) noexcept -> T { return mod(v, m[i]); }, x);
}

template<std::totally_ordered T, usize size>
auto constexpr clamp(vector<T, size> const& x, vector<T, size> const& l, vector<T, size> const& r) noexcept -> vector<T, size> {
    return foreach([&](T const& v, usize i) noexcept -> T { return clamp(v, l[i], r[i]); }, x);
}

template<std::totally_ordered T, usize size>
auto constexpr saturate(vector<T, size> const& x) noexcept -> vector<T, size> {
    return foreach([&](T const& v, usize i) noexcept -> T { return saturate(v); }, x);
}

template<floating_point T, usize size>
auto constexpr lerp(vector<T, size> const& x, vector<T, size> const& y, T const& alpha) noexcept -> vector<T, size> {
    return (T{1.0} - alpha) * x + alpha * y;
}

template<floating_point T, usize size>
auto constexpr lerp(vector<T, size> const& x, vector<T, size> const& y, vector<T, size> const& alpha) noexcept -> vector<T, size> {
    return (T{1.0} - alpha) * x + alpha * y;
}

template<typename T, floating_point U, usize size>
auto constexpr blerp(vector<T, size> const& x, vector<U, size> const& b) noexcept -> T {
    return sum(mul(x, b));
}

template<floating_point T, usize size>
auto constexpr gram_schmidt(vector<T, size> const& y, vector<T, size> const& x) noexcept -> vector<T, size> {
    return y - x * dot(x, y);
}

template<floating_point T, usize size>
auto constexpr orthogonalize(vector<T, size> const& n) noexcept -> matrix<T, 2, size> {
    auto t = abs(n[1]) > T(1) - epsilon<T>
        ? vector<T, 3>{T(1), T(0), T(0)}
        : vector<T, 3>{T(0), T(1), T(0)};
    auto tn = normalize(gram_schmidt(t, n));
    auto bn = normalize(cross(tn, n));
    return {tn, bn};
}

template<typename T, usize size>
auto constexpr sign(vector<T, size> const& x) noexcept -> vector<i32, size> requires floating_point<T> || std::integral<T> {
    return foreach([](T x, auto) { return sign(x); }, x);
}

}

namespace mtt {

#define MTT_VECTOR_ALIAS(p, T) \
    template<usize n> \
    using p##v = core::vector<T, n>; \
    using p##v1 = p##v<1>; \
    using p##v2 = p##v<2>; \
    using p##v3 = p##v<3>; \
    using p##v4 = p##v<4>;

MTT_VECTOR_ALIAS(h, f16)
MTT_VECTOR_ALIAS(f, f32)
MTT_VECTOR_ALIAS(d, f64)
MTT_VECTOR_ALIAS(i, i32)
MTT_VECTOR_ALIAS(u, u32)
MTT_VECTOR_ALIAS(ill, i64)
MTT_VECTOR_ALIAS(ull, u64)
MTT_VECTOR_ALIAS(b, byte)
MTT_VECTOR_ALIAS(uz, usize)

#undef MTT_VECTOR_ALIAS

}
