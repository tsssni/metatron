#pragma once
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/optional.hpp>
#include <algorithm>
#include <array>
#include <span>
#include <tuple>
#include <cassert>

namespace mtt::math {
    // forward declaration to support the declaration of 0d matrix
    template<typename T, usize... dims>
    struct Matrix;

    template<typename T, usize first_dim, usize... rest_dims>
    struct Matrix<T, first_dim, rest_dims...> final {
        using Element = std::conditional_t<sizeof...(rest_dims) == 0, T, Matrix<T, rest_dims...>>;
        auto static constexpr dimensions = std::array<usize, 1 + sizeof...(rest_dims)>{first_dim, rest_dims...};

        constexpr Matrix() noexcept {
            if constexpr (std::is_floating_point_v<Element> || std::is_integral_v<Element>)
                storage.fill(Element{0});
            else
                storage.fill(Element{});
        }

        // directly use Element instead of template type to enable auto inference
        constexpr Matrix(std::initializer_list<Element const> initializer_list) noexcept {
            if (initializer_list.size() > 1)
                std::copy_n(initializer_list.begin(), math::min(first_dim, initializer_list.size()), storage.begin());
            else for (auto& line: storage)
                line = *initializer_list.begin();
        }

        // make convertible elements accepatable by 1d matrix intialization
        template<typename E>
        requires (dimensions.size() == 1uz && std::is_convertible_v<E, Element>)
        constexpr Matrix(std::initializer_list<E const> initializer_list) noexcept {
            if (initializer_list.size() > 1)
                std::copy_n(initializer_list.begin(), math::min(first_dim, initializer_list.size()), storage.begin());
            else for (auto& line: storage)
                line = *initializer_list.begin();
        }

        template<typename E>
        requires std::is_convertible_v<E, Element>
        constexpr Matrix(std::span<E const> initializer_list) noexcept
        {
            if (initializer_list.size() > 1)
                std::copy_n(initializer_list.begin(), math::min(first_dim, initializer_list.size()), storage.begin());
            else for (auto& line: storage)
                line = *initializer_list.begin();
        }

        template<typename U>
        requires std::is_convertible_v<U, T>
        explicit constexpr Matrix(U&& scalar) noexcept {
            if constexpr (dimensions.size() == 1) {
                storage.fill(scalar);
            } else if constexpr (dimensions.size() == 2) {
                auto constexpr diagonal_n = math::min(dimensions[0], dimensions[1]);
                for (auto i = 0; i < diagonal_n; ++i)
                    storage[i][i] = std::forward<U>(scalar);
            } else for (auto& line: storage) {
                line = Element{std::forward<U>(scalar)};
            }
        };

        template<typename... Args>
        requires (true
        && (std::is_convertible_v<Args, T> && ...)
        && dimensions.size() > 1
        && sizeof...(Args) <= math::min(*(dimensions.end() - 2), *(dimensions.end() - 1))
        )
        explicit constexpr Matrix(Args&&... args) noexcept {
            if constexpr (dimensions.size() > 2)
                for (auto& line: storage) line = {args...};
            else
                [this, args...]<usize... idxs>(std::index_sequence<idxs...>) {
                    ((storage[idxs][idxs] = args), ...);
                }(std::make_index_sequence<sizeof...(Args)>{});
        }

        template<typename U, typename... Args, usize rhs_first_dim>
        requires (true
        && std::is_convertible_v<U, T>
        && (std::is_convertible_v<Args, Element> && ...)
        )
        constexpr Matrix(cref<Matrix<U, rhs_first_dim, rest_dims...>> rhs, Args&&... rest) noexcept {
            *this = rhs;
            if constexpr (first_dim > rhs_first_dim)
                [this, &rest...]<usize... idxs>(std::index_sequence<idxs...>) {
                    ((storage[rhs_first_dim + idxs] = std::forward<Args>(rest)), ...);
                }(std::make_index_sequence<math::min(sizeof...(rest), first_dim - rhs_first_dim)>{});
        }

        template<typename U, typename... Args, usize rhs_first_dim>
        requires (true
        && std::is_convertible_v<U, T>
        && (std::is_convertible_v<Args, Element> && ...)
        )
        constexpr Matrix(Matrix<U, rhs_first_dim, rest_dims...>&& rhs, Args&&... rest) noexcept {
            *this = std::move(rhs);
            if constexpr (first_dim > rhs_first_dim)
                [this, &rest...]<usize... idxs>(std::index_sequence<idxs...>) {
                    ((storage[rhs_first_dim + idxs] = std::forward<Args>(rest)), ...);
                }(std::make_index_sequence<math::min(sizeof...(rest), first_dim - rhs_first_dim)>{});
        }

        template<usize rhs_first_dim0, usize rhs_first_dim1>
        constexpr Matrix(
            cref<Matrix<T, rhs_first_dim0, rest_dims...>> rhs0,
            cref<Matrix<T, rhs_first_dim1, rest_dims...>> rhs1
        ) noexcept {
            *this = rhs0;
            if constexpr (first_dim > rhs_first_dim0)
                std::copy_n(rhs1.storage.begin(), math::min(first_dim, rhs_first_dim1) - rhs_first_dim0, storage.begin() + rhs_first_dim0);
        }

        template<usize rhs_first_dim0, usize rhs_first_dim1>
        constexpr Matrix(
            Matrix<T, rhs_first_dim0, rest_dims...>&& rhs0,
            Matrix<T, rhs_first_dim1, rest_dims...>&& rhs1
        ) {
            *this = std::move(rhs0);
            if constexpr (first_dim > rhs_first_dim0)
                std::move(rhs1.storage.begin(), rhs1.storage.begin() + (math::min(first_dim, rhs_first_dim1) - rhs_first_dim0), storage.begin() + rhs_first_dim0);
        }

        template<typename U, usize rhs_first_dim, usize... rhs_rest_dims>
        requires true
        && std::is_convertible_v<U, T>
        && (sizeof...(rest_dims) == sizeof...(rhs_rest_dims))
        auto constexpr operator=(cref<Matrix<U, rhs_first_dim, rhs_rest_dims...>> rhs) noexcept -> ref<Matrix> {
            std::copy_n(rhs.storage.begin(), math::min(first_dim, rhs_first_dim), storage.begin());
            return *this;
        }

        template<typename U, usize rhs_first_dim, usize... rhs_rest_dims>
        requires true
        && std::is_convertible_v<U, T>
        && (sizeof...(rest_dims) == sizeof...(rhs_rest_dims))
        auto constexpr operator=(Matrix<U, rhs_first_dim, rhs_rest_dims...>&& rhs) noexcept -> ref<Matrix> {
            std::move(rhs.storage.begin(), rhs.storage.begin() + math::min(first_dim, rhs_first_dim), storage.begin());
            return *this;
        }

        auto constexpr operator[](usize idx) noexcept -> ref<Element> {
            return storage[idx];
        }

        auto constexpr operator[](usize idx) const noexcept -> cref<Element> {
            return storage[idx];
        }

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
        requires (true
        && longer_n > 1
        // abs and other cmath functions are not constexpr in libstdc++
        && i32(l_n) - i32(r_n) < 2
        && i32(r_n) - i32(l_n) < 2
        && []() noexcept -> bool {return std::equal(
            lds.begin(), lds.begin() + higher_n,
            rds.begin(), rds.begin() + higher_n
        );}()
        && []() noexcept -> bool {
            return lds[higher_n + (l_n > 1 ? 1 : 0)] == rds[higher_n];
        }())
        auto constexpr operator|(
            cref<Matrix<T, rhs_dims...>> rhs
        ) const noexcept {
            using Product_Matrix = decltype([]<usize... dims>(std::index_sequence<dims...>) {
                return Matrix<T, (
                    dims < higher_n ? lds[dims] : dims > higher_n ? rds[higher_n + 1] :
                    (l_n < r_n ? rds[higher_n + 1] : lds[higher_n])
                )...>{};
            }(std::make_index_sequence<shorter_n>{}));
            auto constexpr pds = Product_Matrix::dimensions;
            auto product = Product_Matrix{};

            if constexpr (higher_n > 0) {
                for (auto i = 0; i < first_dim; ++i)
                    product[i] = storage[i] | rhs[i];
            } else {
                using U = Matrix<T, pds.front()>;
                auto constexpr reduce = [](cref<U> x, cref<U> y) -> T {
                    auto z = x * y;
                    T sum = T{0};
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

        auto constexpr operator+(cref<Matrix> rhs) const noexcept -> Matrix {
            auto result = Matrix{};
            for (auto i = 0; i < first_dim; ++i)
                result[i] = storage[i] + rhs[i];
            return result;
        }

        auto constexpr operator+=(cref<Matrix> rhs) noexcept -> ref<Matrix> {
            *this = *this + rhs;
            return *this;
        }

        auto constexpr operator+(cref<T> rhs) const noexcept -> Matrix {
            auto result = Matrix{};
            for (auto i = 0; i < first_dim; ++i)
                result[i] = storage[i] + rhs;
            return result;
        }

        auto constexpr operator+=(cref<T> rhs) noexcept -> ref<Matrix> {
            *this = *this + rhs;
            return *this;
        }

        auto constexpr operator+() noexcept -> Matrix {
            return *this;
        }

        auto constexpr operator-(cref<Matrix> rhs) const noexcept -> Matrix {
            auto result = Matrix{};
            for (auto i = 0; i < first_dim; ++i)
                result[i] = storage[i] - rhs[i];
            return result;
        }

        auto constexpr operator-=(cref<Matrix> rhs) noexcept -> ref<Matrix> {
            *this = *this - rhs;
            return *this;
        }

        auto constexpr operator-(cref<T> rhs) const noexcept -> Matrix {
            auto result = Matrix{};
            for (auto i = 0; i < first_dim; ++i)
                result[i] = storage[i] - rhs;
            return result;
        }

        auto constexpr operator-=(cref<T> rhs) noexcept -> ref<Matrix> {
            *this = *this - rhs;
            return *this;
        }

        auto constexpr operator-() const noexcept -> Matrix {
            auto result = Matrix{};
            for (auto i = 0; i < first_dim; ++i)
                result[i] = -storage[i];
            return result;
        }

        auto constexpr operator*(cref<Matrix> rhs) const noexcept -> Matrix {
            auto result = Matrix{};
            for (auto i = 0; i < first_dim; ++i)
                result[i] = storage[i] * rhs[i];
            return result;
        }

        auto constexpr operator*=(cref<Matrix> rhs) noexcept -> ref<Matrix> {
            *this = *this * rhs;
            return *this;
        }

        auto constexpr operator*(cref<T> rhs) const noexcept -> Matrix {
            auto result = Matrix{};
            for (auto i = 0; i < first_dim; ++i)
                result[i] = storage[i] * rhs;
            return result;
        }

        auto constexpr operator*=(cref<T> rhs) noexcept -> ref<Matrix> {
            *this = *this * rhs;
            return *this;
        }

        auto constexpr operator/(cref<Matrix> rhs) const noexcept -> Matrix {
            auto result = Matrix{};
            for (auto i = 0; i < first_dim; ++i)
                result[i] = math::guarded_div(storage[i], rhs[i]);
            return result;
        }

        auto constexpr operator/=(cref<Matrix> rhs) noexcept -> ref<Matrix> {
            *this = *this / rhs;
            return *this;
        }

        auto constexpr operator/(cref<T> rhs) const noexcept -> Matrix {
            auto result = Matrix{};
            for (auto i = 0; i < first_dim; ++i)
                result[i] = math::guarded_div(storage[i], rhs);
            return result;
        }

        auto constexpr operator/=(cref<T> rhs) noexcept -> ref<Matrix> {
            *this = *this / rhs;
            return *this;
        }

        auto constexpr operator<=>(cref<Matrix> rhs) const = default;

        operator ref<std::array<Element, first_dim>>() {
            return storage;
        };

        operator cref<std::array<Element, first_dim>>() const {
            return storage;
        };

        auto constexpr data() -> mut<T> {
            return mut<T>(&storage);
        };

        auto constexpr data() const -> view<T> {
            return view<T>(&storage);
        };

        auto constexpr size() const -> usize {
            auto s = 1uz;
            for (auto d: dimensions)
                s *= d;
            return s;
        }

        template<usize idx>
        auto constexpr get() const noexcept -> cref<Element> {
            static_assert(idx < first_dim, "index out of bounds");
            return storage[idx];
        }

        template<usize idx>
        auto constexpr get() noexcept -> ref<Element> {
            static_assert(idx < first_dim, "index out of bounds");
            return storage[idx];
        }

    private:
        std::array<Element, first_dim> storage{};

        template<typename U, usize... dims>
        friend struct Matrix;
    };

    template<typename T, usize... dims>
    auto constexpr operator+(cref<T> lhs, cref<Matrix<T, dims...>> rhs) noexcept -> Matrix<T, dims...> {
        return rhs + lhs;
    }

    template<typename T, usize... dims>
    auto constexpr operator-(cref<T> lhs, cref<Matrix<T, dims...>> rhs) noexcept -> Matrix<T, dims...> {
        return -rhs + lhs;
    }

    template<typename T, usize... dims>
    auto constexpr operator*(cref<T> lhs, cref<Matrix<T, dims...>> rhs) noexcept -> Matrix<T, dims...> {
        return rhs * lhs;
    }

    template<typename T, usize... dims>
    auto constexpr operator/(cref<T> lhs, cref<Matrix<T, dims...>> rhs) noexcept -> Matrix<T, dims...> {
        return Matrix<T, dims...>{lhs} / rhs;
    }

    template<usize idx, typename T, usize first_dim, usize... rest_dims>
    auto constexpr get(cref<Matrix<T, first_dim, rest_dims...>> m) 
    noexcept -> cref<typename Matrix<T, first_dim, rest_dims...>::Element> {
        return m.template get<idx>();
    }

    template<usize idx, typename T, usize first_dim, usize... rest_dims>
    auto constexpr get(ref<Matrix<T, first_dim, rest_dims...>> m) 
        noexcept -> ref<typename Matrix<T, first_dim, rest_dims...>::Element> {
        return m.template get<idx>();
    }

    template<typename T, usize h, usize w>
    auto constexpr transpose(cref<Matrix<T, h, w>> m) noexcept -> Matrix<T, w, h> {
        auto result = Matrix<T, w, h>{};
        for (auto i = 0; i < w; ++i)
            for (auto j = 0; j < h; ++j)
                result[i][j] = m[j][i];
        return result;
    }

    template<typename T, usize n>
    requires std::floating_point<T>
    auto constexpr determinant(cref<Matrix<T, n, n>> m) noexcept -> T {
        if constexpr (n == 1) {
            return m[0][0];
        } else if constexpr (n == 2) {
            return m[0][0] * m[1][1] - m[0][1] * m[1][0];
        } else if constexpr (n == 3) {
            return T{0}
            + m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
            - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
            + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
        } else {
            // upper triangular matrix
            auto u = m;
            auto det = T{1};
            
            for (auto i = 0uz; i < n; ++i) {
                auto pivot_row = i;
                auto max_val = math::abs(u[i][i]);
                
                for (auto j = i + 1; j < n; ++j)
                    if (auto curr_val = math::abs(u[j][i]); curr_val > max_val) {
                        max_val = curr_val;
                        pivot_row = j;
                    }
                
                if (max_val < epsilon<T>) return T{0};
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
    }

    template<typename T, usize h>
    requires std::floating_point<T>
    auto constexpr inverse(cref<Matrix<T, h, h>> m) noexcept -> Matrix<T, h, h> {
        auto augmented = Matrix<T, h, h * 2>{};
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

        auto result = Matrix<T, h, h>{};
        for (auto i = 0uz; i < h; ++i)
            for (auto j = 0uz; j < h; ++j)
                result[i][j] = augmented[i][h + j];
        return result;
    }

    template<typename T, usize h, usize w>
    requires std::floating_point<T>
    auto constexpr least_squares(cref<Matrix<T, h, w>> a, cref<Matrix<T, h>> b) noexcept -> Matrix<T, w> {
        auto a_t = math::transpose(a);
        return math::inverse(a_t | a) | (a_t | b);
    }

    template<typename T, usize n>
    requires std::floating_point<T>
    auto constexpr cramer(
        cref<Matrix<T, n, n>> a, 
        cref<Matrix<T, n>> b
    ) noexcept -> opt<Matrix<T, n>> {
        T det_a = determinant(a);
        if (math::abs(det_a) < epsilon<T>) return {};

        auto result = Matrix<T, n>{};
        for (auto i = 0uz; i < n; ++i) {
            auto a_i = a;
            for (auto j = 0uz; j < n; ++j)
                a_i[j][i] = b[j];
            result[i] = determinant(a_i) / det_a;
        }
        
        return result;
    }

    template<typename T, usize n, usize m>
    requires std::floating_point<T>
    auto constexpr cramer(
        cref<Matrix<T, n, n>> a, 
        cref<Matrix<T, n, m>> b
    ) noexcept -> opt<Matrix<T, n, m>> {
        T det_a = determinant(a);
        if (math::abs(det_a) < 1e-9) return {};

        auto result = Matrix<T, n, m>{};
        if constexpr (n == 2) {
            result[0] = (a[1][1] * b[0] - a[0][1] * b[1]) / det_a;
            result[1] = (a[0][0] * b[1] - a[1][0] * b[0]) / det_a;
        } else {
            for (auto i = 0uz; i < n; ++i) {
                for (auto j = 0uz; j < m; ++j) {
                    auto a_i = a;
                    for (auto k = 0uz; k < n; ++k)
                        a_i[k][i] = b[k][j];
                    result[i][j] = determinant(a_i) / det_a;
                }
            }
        }
        
        return result;
    }
}

namespace std {
    template<typename T, mtt::usize first_dim, mtt::usize... rest_dims>
    struct tuple_size<mtt::math::Matrix<T, first_dim, rest_dims...>> 
        : std::integral_constant<size_t, first_dim> {};

    template<mtt::usize I, typename T, mtt::usize first_dim, mtt::usize... rest_dims>
    struct tuple_element<I, mtt::math::Matrix<T, first_dim, rest_dims...>> {
        using type = typename mtt::math::Matrix<T, first_dim, rest_dims...>::Element;
    };
}

namespace mtt {
    #define MTT_MATRIX_ALIAS(p, T)\
    template<usize... dims>\
    using p##m = math::Matrix<T, dims...>;\
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

    MTT_MATRIX_ALIAS(f, f32)
    MTT_MATRIX_ALIAS(d, f64)
    MTT_MATRIX_ALIAS(i, i32)
    MTT_MATRIX_ALIAS(u, u32)
    MTT_MATRIX_ALIAS(ill, i64)
    MTT_MATRIX_ALIAS(ull, u64)
    MTT_MATRIX_ALIAS(b, byte)
    MTT_MATRIX_ALIAS(uz, usize)
}
