#pragma once
#include <metatron/core/stl/concept.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <array>
#include <span>
#include <tuple>

namespace mtt::math {

// forward declaration to support the declaration of 0d matrix
template<typename T, usize... dims>
struct matrix;

template<typename T, usize dim0, usize... rest>
struct matrix<T, dim0, rest...> final {
    using Element = std::conditional_t<sizeof...(rest) == 0, T, matrix<T, rest...>>;
    auto constexpr static dimensions = std::array<usize, 1 + sizeof...(rest)>{dim0, rest...};

    constexpr matrix() noexcept;

    // directly use Element instead of template type to enable auto inference
    constexpr matrix(std::initializer_list<Element const> initializer_list) noexcept;

    // make convertible elements accepatable by 1d matrix intialization
    template<typename E>
    constexpr matrix(std::initializer_list<E const> initializer_list) noexcept
    requires (dimensions.size() == 1uz && stl::convertible_to<E, Element>);

    template<typename E>
    constexpr matrix(std::span<E const> initializer_list) noexcept
    requires stl::convertible_to<E, Element>;

    template<typename U>
    explicit constexpr matrix(U&& scalar) noexcept
    requires stl::convertible_to<U, T>;

    template<typename... Args>
    explicit constexpr matrix(Args&&... args) noexcept
    requires (true
    && (stl::convertible_to<Args, T> && ...) && dimensions.size() > 1
    && sizeof...(Args) <= math::min(*(dimensions.end() - 2), *(dimensions.end() - 1)));

    template<typename U, typename... Args, usize rhs_dim0>
    constexpr matrix(matrix<U, rhs_dim0, rest...> const& rhs, Args&&... args) noexcept
    requires (stl::convertible_to<U, T> && (stl::convertible_to<Args, Element> && ...));
    template<typename U, typename... Args, usize rhs_dim0>
    constexpr matrix(matrix<U, rhs_dim0, rest...>&& rhs, Args&&... args) noexcept
    requires (stl::convertible_to<U, T> && (stl::convertible_to<Args, Element> && ...));

    template<usize rhs_dim0, usize rhs_dim1>
    constexpr matrix(matrix<T, rhs_dim0, rest...> const& rhs0, matrix<T, rhs_dim1, rest...> const& rhs1) noexcept;
    template<usize rhs_dim0, usize rhs_dim1>
    constexpr matrix(matrix<T, rhs_dim0, rest...>&& rhs0, matrix<T, rhs_dim1, rest...>&& rhs1) noexcept;

    template<typename U, usize rhs_dim0, usize... rhs_rest>
    auto constexpr operator=(matrix<U, rhs_dim0, rhs_rest...> const& rhs) noexcept -> matrix&
    requires stl::convertible_to<U, T> && (sizeof...(rest) == sizeof...(rhs_rest));
    template<typename U, usize rhs_dim0, usize... rhs_rest>
    auto constexpr operator=(matrix<U, rhs_dim0, rhs_rest...>&& rhs) noexcept -> matrix&

    requires stl::convertible_to<U, T> && (sizeof...(rest) == sizeof...(rhs_rest));
    auto constexpr operator[](usize idx) noexcept -> Element&;
    auto constexpr operator[](usize idx) const noexcept -> Element const&;

    template<
        usize... rhs_dims,
        usize l_n = dimensions.size(),
        usize r_n = sizeof...(rhs_dims),
        usize shorter_n = math::min(l_n, r_n),
        usize longer_n = math::max(l_n, r_n),
        usize higher_n = math::max(usize(0), longer_n - 2),
        std::array<usize, l_n> lds = dimensions,
        std::array<usize, r_n> rds = {rhs_dims...}
    >
    auto constexpr operator|(matrix<T, rhs_dims...> const& rhs) const noexcept
    requires (true
    && longer_n > 1 && longer_n - shorter_n < 2
    && std::equal(lds.begin(), lds.begin() + higher_n, rds.begin(), rds.begin() + higher_n)
    && lds[higher_n + (l_n > 1 ? 1 : 0)] == rds[higher_n]);

    auto constexpr operator+(matrix const& rhs) const noexcept -> matrix;
    auto constexpr operator+=(matrix const& rhs) noexcept -> matrix&;
    auto constexpr operator+(T const& rhs) const noexcept -> matrix;
    auto constexpr operator+=(T const& rhs) noexcept -> matrix&;
    auto constexpr operator+() noexcept -> matrix;

    auto constexpr operator-(matrix const& rhs) const noexcept -> matrix;
    auto constexpr operator-=(matrix const& rhs) noexcept -> matrix&;
    auto constexpr operator-(T const& rhs) const noexcept -> matrix;
    auto constexpr operator-=(T const& rhs) noexcept -> matrix&;
    auto constexpr operator-() const noexcept -> matrix;

    auto constexpr operator*(matrix const& rhs) const noexcept -> matrix;
    auto constexpr operator*=(matrix const& rhs) noexcept -> matrix&;
    auto constexpr operator*(T const& rhs) const noexcept -> matrix;
    auto constexpr operator*=(T const& rhs) noexcept -> matrix&;

    auto constexpr operator/(matrix const& rhs) const noexcept -> matrix;
    auto constexpr operator/=(matrix const& rhs) noexcept -> matrix&;
    auto constexpr operator/(T const& rhs) const noexcept -> matrix;
    auto constexpr operator/=(T const& rhs) noexcept -> matrix&;

    auto constexpr operator<=>(matrix const& rhs) const = default;

    operator std::span<Element, dim0>();
    operator std::span<Element const, dim0>() const;

    auto constexpr data() -> T*;
    auto constexpr data() const -> T const*;
    auto constexpr size() const -> usize;
    template<usize idx>
    auto constexpr get() const noexcept -> Element const& requires (idx < dim0);
    template<usize idx>
    auto constexpr get() noexcept -> Element& requires (idx < dim0);

private:
    std::array<Element, dim0> storage{};

    template<typename U, usize... dims>
    friend struct matrix;
};

template<typename T, usize dim0, usize... rest>
constexpr matrix<T, dim0, rest...>::matrix() noexcept {
    if constexpr (stl::floating_point<Element> || stl::integral<Element>)
        storage.fill(Element{0});
    else
        storage.fill(Element{});
}

template<typename T, usize dim0, usize... rest>
constexpr matrix<T, dim0, rest...>::matrix(std::initializer_list<Element const> initializer_list) noexcept {
    if (initializer_list.size() > 1)
        std::copy_n(initializer_list.begin(), math::min(dim0, initializer_list.size()), storage.begin());
    else for (auto& line: storage)
        line = *initializer_list.begin();
}

template<typename T, usize dim0, usize... rest>
template<typename E>
constexpr matrix<T, dim0, rest...>::matrix(std::initializer_list<E const> initializer_list) noexcept
requires (dimensions.size() == 1uz && stl::convertible_to<E, Element>) {
    if (initializer_list.size() > 1)
        std::copy_n(initializer_list.begin(), math::min(dim0, initializer_list.size()), storage.begin());
    else for (auto& line: storage)
        line = *initializer_list.begin();
}

template<typename T, usize dim0, usize... rest>
template<typename E>
constexpr matrix<T, dim0, rest...>::matrix(std::span<E const> initializer_list) noexcept
requires stl::convertible_to<E, Element> {
    if (initializer_list.size() > 1)
        std::copy_n(initializer_list.begin(), math::min(dim0, initializer_list.size()), storage.begin());
    else for (auto& line: storage)
        line = *initializer_list.begin();
}

template<typename T, usize dim0, usize... rest>
template<typename U>
constexpr matrix<T, dim0, rest...>::matrix(U&& scalar) noexcept
requires stl::convertible_to<U, T> {
    if constexpr (dimensions.size() == 1) {
        storage.fill(scalar);
    } else if constexpr (dimensions.size() == 2) {
        auto constexpr diagonal_n = math::min(dimensions[0], dimensions[1]);
        for (auto i = 0; i < diagonal_n; ++i)
            storage[i][i] = std::forward<U>(scalar);
    } else for (auto& line: storage) {
        line = Element{std::forward<U>(scalar)};
    }
}

template<typename T, usize dim0, usize... rest>
template<typename... Args>
constexpr matrix<T, dim0, rest...>::matrix(Args&&... args) noexcept
requires (true
&& (stl::convertible_to<Args, T> && ...) && dimensions.size() > 1
&& sizeof...(Args) <= math::min(*(dimensions.end() - 2), *(dimensions.end() - 1))) {
    if constexpr (dimensions.size() > 2)
        for (auto& line: storage) line = {args...};
    else
        [this, args...]<usize... idxs>(std::index_sequence<idxs...>) {
            ((storage[idxs][idxs] = args), ...);
        }(std::make_index_sequence<sizeof...(Args)>{});
}

template<typename T, usize dim0, usize... rest>
template<typename U, typename... Args, usize rhs_dim0>
constexpr matrix<T, dim0, rest...>::matrix(matrix<U, rhs_dim0, rest...> const& rhs, Args&&... args) noexcept
requires (stl::convertible_to<U, T> && (stl::convertible_to<Args, Element> && ...)) {
    *this = rhs;
    if constexpr (dim0 > rhs_dim0)
        [this, &args...]<usize... idxs>(std::index_sequence<idxs...>) {
            ((storage[rhs_dim0 + idxs] = std::forward<Args>(args)), ...);
        }(std::make_index_sequence<math::min(sizeof...(args), dim0 - rhs_dim0)>{});
}
template<typename T, usize dim0, usize... rest>
template<typename U, typename... Args, usize rhs_dim0>
constexpr matrix<T, dim0, rest...>::matrix(matrix<U, rhs_dim0, rest...>&& rhs, Args&&... args) noexcept
requires (stl::convertible_to<U, T> && (stl::convertible_to<Args, Element> && ...)) {
    *this = std::move(rhs);
    if constexpr (dim0 > rhs_dim0)
        [this, &args...]<usize... idxs>(std::index_sequence<idxs...>) {
            ((storage[rhs_dim0 + idxs] = std::forward<Args>(args)), ...);
        }(std::make_index_sequence<math::min(sizeof...(args), dim0 - rhs_dim0)>{});
}

template<typename T, usize dim0, usize... rest>
template<usize rhs_dim0, usize rhs_dim1>
constexpr matrix<T, dim0, rest...>::matrix(matrix<T, rhs_dim0, rest...> const& rhs0, matrix<T, rhs_dim1, rest...> const& rhs1) noexcept {
    *this = rhs0;
    if constexpr (dim0 > rhs_dim0)
        std::copy_n(rhs1.storage.begin(), math::min(dim0 - rhs_dim0, rhs_dim1), storage.begin() + rhs_dim0);
}
template<typename T, usize dim0, usize... rest>
template<usize rhs_dim0, usize rhs_dim1>
constexpr matrix<T, dim0, rest...>::matrix(matrix<T, rhs_dim0, rest...>&& rhs0, matrix<T, rhs_dim1, rest...>&& rhs1) noexcept {
    *this = std::move(rhs0);
    if constexpr (dim0 > rhs_dim0)
        std::move(rhs1.storage.begin(), rhs1.storage.begin() + math::min(dim0 - rhs_dim0, rhs_dim1), storage.begin() + rhs_dim0);
}

template<typename T, usize dim0, usize... rest>
template<typename U, usize rhs_dim0, usize... rhs_rest>
auto constexpr matrix<T, dim0, rest...>::operator=(matrix<U, rhs_dim0, rhs_rest...> const& rhs) noexcept -> matrix&
requires stl::convertible_to<U, T> && (sizeof...(rest) == sizeof...(rhs_rest)) {
    std::copy_n(rhs.storage.begin(), math::min(dim0, rhs_dim0), storage.begin());
    return *this;
}
template<typename T, usize dim0, usize... rest>
template<typename U, usize rhs_dim0, usize... rhs_rest>
auto constexpr matrix<T, dim0, rest...>::operator=(matrix<U, rhs_dim0, rhs_rest...>&& rhs) noexcept -> matrix&
requires stl::convertible_to<U, T> && (sizeof...(rest) == sizeof...(rhs_rest)) {
    std::move(rhs.storage.begin(), rhs.storage.begin() + math::min(dim0, rhs_dim0), storage.begin());
    return *this;
}

template<typename T, usize dim0, usize... rest>
auto constexpr matrix<T, dim0, rest...>::operator[](usize idx) noexcept -> Element& { return storage[idx]; }
template<typename T, usize dim0, usize... rest>
auto constexpr matrix<T, dim0, rest...>::operator[](usize idx) const noexcept -> Element const& { return storage[idx]; }

template<typename T, usize dim0, usize... rest>
template<
    usize... rhs_dims, usize l_n, usize r_n,
    usize shorter_n, usize longer_n, usize higher_n,
    std::array<usize, l_n> lds, std::array<usize, r_n> rds
>
auto constexpr matrix<T, dim0, rest...>::operator|(matrix<T, rhs_dims...> const& rhs) const noexcept
requires (true
&& longer_n > 1 && longer_n - shorter_n < 2
&& std::equal(lds.begin(), lds.begin() + higher_n, rds.begin(), rds.begin() + higher_n)
&& lds[higher_n + (l_n > 1 ? 1 : 0)] == rds[higher_n]) {
    using Product_Matrix = decltype([]<usize... dims>(std::index_sequence<dims...>) {
        return matrix<T, (
            dims < higher_n ? lds[dims] : dims > higher_n ? rds[higher_n + 1] :
            (l_n < r_n ? rds[higher_n + 1] : lds[higher_n])
        )...>{};
    }(std::make_index_sequence<shorter_n>{}));
    auto constexpr pds = Product_Matrix::dimensions;
    auto product = Product_Matrix{};

    if constexpr (higher_n > 0) {
        for (auto i = 0; i < dim0; ++i)
            product[i] = storage[i] | rhs[i];
    } else {
        using U = matrix<T, pds.front()>;
        auto constexpr reduce = [](U const& x, U const& y) -> T {
            auto z = x * y;
            T sum = T(0);
            for (auto i = 0uz; i < U::dimensions.front(); ++i)
                sum += z[i];
            return sum;
        };

        if constexpr (r_n == 1) {
            for (auto i = 0; i < pds[0]; ++i)
                product[i] = reduce((*this)[i], rhs);
        } else if constexpr (l_n == 1) {
            auto rt = transpose(rhs);
            for (auto i = 0; i < pds[0]; ++i)
                product[i] = reduce(*this, transpose(rhs)[i]);
        } else {
            for (auto i = 0; i < pds[0]; ++i)
                for (auto j = 0; j < pds[1]; ++j)
                    product[i][j] = reduce((*this)[i], transpose(rhs)[j]);
        }
    }

    return product;
}

template<typename T, usize dim0, usize... rest>
auto constexpr matrix<T, dim0, rest...>::operator+(matrix const& rhs) const noexcept -> matrix { auto m = matrix{}; for (auto i = 0; i < dim0; ++i) m[i] = storage[i] + rhs[i]; return m; }
template<typename T, usize dim0, usize... rest>
auto constexpr matrix<T, dim0, rest...>::operator+=(matrix const& rhs) noexcept -> matrix& { *this = *this + rhs; return *this; }
template<typename T, usize dim0, usize... rest>
auto constexpr matrix<T, dim0, rest...>::operator+(T const& rhs) const noexcept -> matrix { auto m = matrix{}; for (auto i = 0; i < dim0; ++i) m[i] = storage[i] + rhs; return m; }
template<typename T, usize dim0, usize... rest>
auto constexpr matrix<T, dim0, rest...>::operator+=(T const& rhs) noexcept -> matrix& { *this = *this + rhs; return *this; }
template<typename T, usize dim0, usize... rest>
auto constexpr matrix<T, dim0, rest...>::operator+() noexcept -> matrix { return *this; }

template<typename T, usize dim0, usize... rest>
auto constexpr matrix<T, dim0, rest...>::operator-(matrix const& rhs) const noexcept -> matrix { auto m = matrix{}; for (auto i = 0; i < dim0; ++i) m[i] = storage[i] - rhs[i]; return m; }
template<typename T, usize dim0, usize... rest>
auto constexpr matrix<T, dim0, rest...>::operator-=(matrix const& rhs) noexcept -> matrix& { *this = *this - rhs; return *this; }
template<typename T, usize dim0, usize... rest>
auto constexpr matrix<T, dim0, rest...>::operator-(T const& rhs) const noexcept -> matrix { auto m = matrix{}; for (auto i = 0; i < dim0; ++i) m[i] = storage[i] - rhs; return m; }
template<typename T, usize dim0, usize... rest>
auto constexpr matrix<T, dim0, rest...>::operator-=(T const& rhs) noexcept -> matrix& { *this = *this - rhs; return *this; }
template<typename T, usize dim0, usize... rest>
auto constexpr matrix<T, dim0, rest...>::operator-() const noexcept -> matrix { auto m = matrix{}; for (auto i = 0; i < dim0; ++i) m[i] = -storage[i]; return m; }

template<typename T, usize dim0, usize... rest>
auto constexpr matrix<T, dim0, rest...>::operator*(matrix const& rhs) const noexcept -> matrix { auto m = matrix{}; for (auto i = 0; i < dim0; ++i) m[i] = storage[i] * rhs[i]; return m; }
template<typename T, usize dim0, usize... rest>
auto constexpr matrix<T, dim0, rest...>::operator*=(matrix const& rhs) noexcept -> matrix& { *this = *this * rhs; return *this; }
template<typename T, usize dim0, usize... rest>
auto constexpr matrix<T, dim0, rest...>::operator*(T const& rhs) const noexcept -> matrix { auto m = matrix{}; for (auto i = 0; i < dim0; ++i) m[i] = storage[i] * rhs; return m; }
template<typename T, usize dim0, usize... rest>
auto constexpr matrix<T, dim0, rest...>::operator*=(T const& rhs) noexcept -> matrix& { *this = *this * rhs; return *this; }

template<typename T, usize dim0, usize... rest>
auto constexpr matrix<T, dim0, rest...>::operator/(matrix const& rhs) const noexcept -> matrix { auto m = matrix{}; for (auto i = 0; i < dim0; ++i) m[i] = storage[i] / rhs[i]; return m; }
template<typename T, usize dim0, usize... rest>
auto constexpr matrix<T, dim0, rest...>::operator/=(matrix const& rhs) noexcept -> matrix& { *this = *this / rhs; return *this; }
template<typename T, usize dim0, usize... rest>
auto constexpr matrix<T, dim0, rest...>::operator/(T const& rhs) const noexcept -> matrix { auto m = matrix{}; for (auto i = 0; i < dim0; ++i) m[i] = storage[i] / rhs; return m; }
template<typename T, usize dim0, usize... rest>
auto constexpr matrix<T, dim0, rest...>::operator/=(T const& rhs) noexcept -> matrix& { *this = *this / rhs; return *this; }

template<typename T, usize dim0, usize... rest>
matrix<T, dim0, rest...>::operator std::span<Element, dim0>() { return storage; }
template<typename T, usize dim0, usize... rest>
matrix<T, dim0, rest...>::operator std::span<Element const, dim0>() const { return storage; }

template<typename T, usize dim0, usize... rest>
auto constexpr matrix<T, dim0, rest...>::data() -> T* { return (T*)&storage; }
template<typename T, usize dim0, usize... rest>
auto constexpr matrix<T, dim0, rest...>::data() const -> T const* { return (T const*)&storage; }
template<typename T, usize dim0, usize... rest>
auto constexpr matrix<T, dim0, rest...>::size() const -> usize { auto s = 1uz; for (auto d: dimensions) s *= d; return s; }
template<typename T, usize dim0, usize... rest>
template<usize idx>
auto constexpr matrix<T, dim0, rest...>::get() const noexcept -> Element const& requires (idx < dim0) { return storage[idx]; }
template<typename T, usize dim0, usize... rest>
template<usize idx>
auto constexpr matrix<T, dim0, rest...>::get() noexcept -> Element& requires (idx < dim0) { return storage[idx]; }

template<typename T, usize... dims>
auto constexpr operator+(T const& lhs, matrix<T, dims...> const& rhs) noexcept -> matrix<T, dims...> { return rhs + lhs; }
template<typename T, usize... dims>
auto constexpr operator-(T const& lhs, matrix<T, dims...> const& rhs) noexcept -> matrix<T, dims...> { return -rhs + lhs; }
template<typename T, usize... dims>
auto constexpr operator*(T const& lhs, matrix<T, dims...> const& rhs) noexcept -> matrix<T, dims...> { return rhs * lhs; }
template<typename T, usize... dims>
auto constexpr operator/(T const& lhs, matrix<T, dims...> const& rhs) noexcept -> matrix<T, dims...> { return matrix<T, dims...>{lhs} / rhs; }

template<usize idx, typename T, usize dim0, usize... rest>
auto constexpr get(matrix<T, dim0, rest...> const& m) noexcept -> typename matrix<T, dim0, rest...>::Element const& requires (idx < dim0) { return m.template get<idx>(); }
template<usize idx, typename T, usize dim0, usize... rest>
auto constexpr get(matrix<T, dim0, rest...>& m) noexcept -> typename matrix<T, dim0, rest...>::Element& requires (idx < dim0) { return m.template get<idx>(); }

template<typename T, usize h, usize w>
auto constexpr transpose(matrix<T, h, w> const& m) noexcept -> matrix<T, w, h> {
    auto result = matrix<T, w, h>{};
    for (auto i = 0; i < w; ++i)
        for (auto j = 0; j < h; ++j)
            result[i][j] = m[j][i];
    return result;
}

template<stl::floating_point T, usize n>
auto constexpr determinant(matrix<T, n, n> const& m) noexcept -> T requires (n == 1) { return m[0][0]; }

template<stl::floating_point T, usize n>
auto constexpr determinant(matrix<T, n, n> const& m) noexcept -> T requires (n == 2) { return m[0][0] * m[1][1] - m[0][1] * m[1][0]; }

template<stl::floating_point T, usize n>
auto constexpr determinant(matrix<T, n, n> const& m) noexcept -> T
requires (n == 3) {
    return T(0)
    + m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
    - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
    + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}

template<stl::floating_point T, usize n>
auto constexpr determinant(matrix<T, n, n> const& m) noexcept -> T
requires (n > 3) {
    // upper triangular matrix
    auto u = m;
    auto det = T(1);

    for (auto i = 0uz; i < n; ++i) {
        auto pivot_row = i;
        auto max_val = math::abs(u[i][i]);

        for (auto j = i + 1; j < n; ++j)
            if (auto curr_val = math::abs(u[j][i]); curr_val > max_val) {
                max_val = curr_val;
                pivot_row = j;
            }

        if (max_val < epsilon<T>) return T(0);
        if (pivot_row != i) {
            std::swap(u[i], u[pivot_row]);
            det = -det;
        }
        det *= u[i][i];

        for (auto j = i + 1; j < n; ++j) {
            auto factor = u[j][i] / u[i][i];
            for (auto k = i + 1; k < n; ++k)
                u[j][k] -= factor * u[i][k];
        }
    }

    return det;
}

template<stl::floating_point T, usize h>
auto constexpr inverse(matrix<T, h, h> const& m) noexcept -> matrix<T, h, h> requires (h == 1) { return matrix<T, h, h>(T(1) / m[0][0]); }

template<stl::floating_point T, usize h>
auto constexpr inverse(matrix<T, h, h> const& m) noexcept -> matrix<T, h, h>
requires (h == 2) {
    auto det = m[0][0] * m[1][1] - m[0][1] * m[1][0];
    auto inv_det = math::guarded_div(T(1), det);

    auto result = matrix<T, 2, 2>{};
    result[0][0] = m[1][1] * inv_det;
    result[0][1] = -m[0][1] * inv_det;
    result[1][0] = -m[1][0] * inv_det;
    result[1][1] = m[0][0] * inv_det;
    return result;
}

template<stl::floating_point T, usize h>
auto constexpr inverse(matrix<T, h, h> const& m) noexcept -> matrix<T, h, h>
requires (h == 3) {
    auto c00 = m[1][1] * m[2][2] - m[1][2] * m[2][1];
    auto c01 = -(m[1][0] * m[2][2] - m[1][2] * m[2][0]);
    auto c02 = m[1][0] * m[2][1] - m[1][1] * m[2][0];

    auto c10 = -(m[0][1] * m[2][2] - m[0][2] * m[2][1]);
    auto c11 = m[0][0] * m[2][2] - m[0][2] * m[2][0];
    auto c12 = -(m[0][0] * m[2][1] - m[0][1] * m[2][0]);

    auto c20 = m[0][1] * m[1][2] - m[0][2] * m[1][1];
    auto c21 = -(m[0][0] * m[1][2] - m[0][2] * m[1][0]);
    auto c22 = m[0][0] * m[1][1] - m[0][1] * m[1][0];

    auto det = m[0][0] * c00 + m[0][1] * c01 + m[0][2] * c02;
    auto inv_det = math::guarded_div(T(1), det);

    auto result = matrix<T, 3, 3>{};
    result[0][0] = c00 * inv_det;
    result[0][1] = c10 * inv_det;
    result[0][2] = c20 * inv_det;
    result[1][0] = c01 * inv_det;
    result[1][1] = c11 * inv_det;
    result[1][2] = c21 * inv_det;
    result[2][0] = c02 * inv_det;
    result[2][1] = c12 * inv_det;
    result[2][2] = c22 * inv_det;
    return result;
}

template<stl::floating_point T, usize h>
auto constexpr inverse(matrix<T, h, h> const& m) noexcept -> matrix<T, h, h>
requires (h == 4) {
    auto s0 = m[0][0] * m[1][1] - m[1][0] * m[0][1];
    auto s1 = m[0][0] * m[1][2] - m[1][0] * m[0][2];
    auto s2 = m[0][0] * m[1][3] - m[1][0] * m[0][3];
    auto s3 = m[0][1] * m[1][2] - m[1][1] * m[0][2];
    auto s4 = m[0][1] * m[1][3] - m[1][1] * m[0][3];
    auto s5 = m[0][2] * m[1][3] - m[1][2] * m[0][3];

    auto c5 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
    auto c4 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
    auto c3 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
    auto c2 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
    auto c1 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
    auto c0 = m[2][0] * m[3][1] - m[3][0] * m[2][1];

    auto det = s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
    auto inv_det = math::guarded_div(T(1), det);

    auto result = matrix<T, 4, 4>{};
    result[0][0] = (m[1][1] * c5 - m[1][2] * c4 + m[1][3] * c3) * inv_det;
    result[0][1] = (-m[0][1] * c5 + m[0][2] * c4 - m[0][3] * c3) * inv_det;
    result[0][2] = (m[3][1] * s5 - m[3][2] * s4 + m[3][3] * s3) * inv_det;
    result[0][3] = (-m[2][1] * s5 + m[2][2] * s4 - m[2][3] * s3) * inv_det;

    result[1][0] = (-m[1][0] * c5 + m[1][2] * c2 - m[1][3] * c1) * inv_det;
    result[1][1] = (m[0][0] * c5 - m[0][2] * c2 + m[0][3] * c1) * inv_det;
    result[1][2] = (-m[3][0] * s5 + m[3][2] * s2 - m[3][3] * s1) * inv_det;
    result[1][3] = (m[2][0] * s5 - m[2][2] * s2 + m[2][3] * s1) * inv_det;

    result[2][0] = (m[1][0] * c4 - m[1][1] * c2 + m[1][3] * c0) * inv_det;
    result[2][1] = (-m[0][0] * c4 + m[0][1] * c2 - m[0][3] * c0) * inv_det;
    result[2][2] = (m[3][0] * s4 - m[3][1] * s2 + m[3][3] * s0) * inv_det;
    result[2][3] = (-m[2][0] * s4 + m[2][1] * s2 - m[2][3] * s0) * inv_det;

    result[3][0] = (-m[1][0] * c3 + m[1][1] * c1 - m[1][2] * c0) * inv_det;
    result[3][1] = (m[0][0] * c3 - m[0][1] * c1 + m[0][2] * c0) * inv_det;
    result[3][2] = (-m[3][0] * s3 + m[3][1] * s1 - m[3][2] * s0) * inv_det;
    result[3][3] = (m[2][0] * s3 - m[2][1] * s1 + m[2][2] * s0) * inv_det;

    return result;
}

template<stl::floating_point T, usize h>
auto constexpr inverse(matrix<T, h, h> const& m) noexcept -> matrix<T, h, h>
requires (h > 4) {
    auto augmented = matrix<T, h, h * 2>{};
    for (auto i = 0uz; i < h; ++i)
        for (auto j = 0uz; j < h; ++j)
            augmented[i][j] = m[i][j];
    for (auto i = 0uz; i < h; ++i)
        augmented[i][h + i] = T(1);

    // Gaussian-Jordan
    for (auto i = 0uz; i < h; ++i) {
        auto pivot_row = i;
        auto max_val = math::abs(augmented[i][i]);

        for (auto j = i + 1; j < h; ++j)
            if (auto curr_val = math::abs(augmented[j][i]); curr_val > max_val) {
                max_val = curr_val;
                pivot_row = j;
            }

        if (pivot_row != i) std::swap(augmented[i], augmented[pivot_row]);
        auto pivot = augmented[i][i];
        augmented[i] /= pivot;

        for (auto j = 0uz; j < h; ++j)
            if (j != i) {
                auto factor = augmented[j][i];
                augmented[j] -= factor * augmented[i];
            }
    }

    auto result = matrix<T, h, h>{};
    for (auto i = 0uz; i < h; ++i)
        for (auto j = 0uz; j < h; ++j)
            result[i][j] = augmented[i][h + j];
    return result;
}

template<stl::floating_point T, usize h, usize w>
auto constexpr least_squares(matrix<T, h, w> const& a, matrix<T, h> const& b) noexcept -> matrix<T, w> {
    auto a_t = math::transpose(a);
    return math::inverse(a_t | a) | (a_t | b);
}

template<stl::floating_point T, usize n>
auto constexpr cramer(
    matrix<T, n, n> const& a,
    matrix<T, n> const& b
) noexcept -> matrix<T, n> {
    T det_a = determinant(a);
    auto result = matrix<T, n>{};
    for (auto i = 0uz; i < n; ++i) {
        auto a_i = a;
        for (auto j = 0uz; j < n; ++j)
            a_i[j][i] = b[j];
        result[i] = determinant(a_i) / det_a;
    }

    return result;
}

template<stl::floating_point T, usize n, usize m>
auto constexpr cramer(
    matrix<T, n, n> const& a,
    matrix<T, n, m> const& b
) noexcept -> matrix<T, n, m>
requires (n == 2) {
    T det_a = determinant(a);
    auto result = matrix<T, n, m>{};
    result[0] = (a[1][1] * b[0] - a[0][1] * b[1]) / det_a;
    result[1] = (a[0][0] * b[1] - a[1][0] * b[0]) / det_a;

    return result;
}

template<stl::floating_point T, usize n, usize m>
auto constexpr cramer(
    matrix<T, n, n> const& a,
    matrix<T, n, m> const& b
) noexcept -> matrix<T, n, m>
requires (n != 2) {
    T det_a = determinant(a);
    auto result = matrix<T, n, m>{};
    for (auto i = 0uz; i < n; ++i) {
        for (auto j = 0uz; j < m; ++j) {
            auto a_i = a;
            for (auto k = 0uz; k < n; ++k)
                a_i[k][i] = b[k][j];
            result[i][j] = determinant(a_i) / det_a;
        }
    }

    return result;
}

}

namespace std {

template<typename T, mtt::usize dim0, mtt::usize... rest>
struct tuple_size<mtt::math::matrix<T, dim0, rest...>>:
std::integral_constant<size_t, dim0> {};

template<mtt::usize I, typename T, mtt::usize dim0, mtt::usize... rest>
struct tuple_element<I, mtt::math::matrix<T, dim0, rest...>> {
    using type = typename mtt::math::matrix<T, dim0, rest...>::Element;
};

}

namespace mtt {

#define MTT_MATRIX_ALIAS(p, T)\
template<usize... dims>\
using p##m = math::matrix<T, dims...>;\
\
using p##m11 = p##m<1, 1>;\
using p##m12 = p##m<1, 2>;\
using p##m13 = p##m<1, 3>;\
using p##m14 = p##m<1, 4>;\
\
using p##m21 = p##m<2, 1>;\
using p##m22 = p##m<2, 2>;\
using p##m23 = p##m<2, 3>;\
using p##m24 = p##m<2, 4>;\
\
using p##m31 = p##m<3, 1>;\
using p##m32 = p##m<3, 2>;\
using p##m33 = p##m<3, 3>;\
using p##m34 = p##m<3, 4>;\
\
using p##m41 = p##m<4, 1>;\
using p##m42 = p##m<4, 2>;\
using p##m43 = p##m<4, 3>;\
using p##m44 = p##m<4, 4>;\
\
using p##m1 = p##m11;\
using p##m2 = p##m22;\
using p##m3 = p##m33;\
using p##m4 = p##m44;

MTT_MATRIX_ALIAS(h, f16)
MTT_MATRIX_ALIAS(f, f32)
MTT_MATRIX_ALIAS(d, f64)
MTT_MATRIX_ALIAS(i, i32)
MTT_MATRIX_ALIAS(u, u32)
MTT_MATRIX_ALIAS(ill, i64)
MTT_MATRIX_ALIAS(ull, u64)
MTT_MATRIX_ALIAS(b, byte)
MTT_MATRIX_ALIAS(uz, usize)

#undef MTT_MATRIX_ALIAS

}
