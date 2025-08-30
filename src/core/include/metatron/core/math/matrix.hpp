#pragma once
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/arithmetic.hpp>
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
	requires (first_dim > 0 && (... && (rest_dims > 0)))
	struct Matrix<T, first_dim, rest_dims...> final {
		using Element = std::conditional_t<sizeof...(rest_dims) == 0, T, Matrix<T, rest_dims...>>;
		auto static constexpr dimensions = std::array<usize, 1 + sizeof...(rest_dims)>{first_dim, rest_dims...};

		Matrix() noexcept = default;

		// directly use Element instead of template type to enable auto inference
		constexpr Matrix(std::initializer_list<Element const> initializer_list) noexcept {
			if (initializer_list.size() > 1) {
				std::copy_n(initializer_list.begin(), std::min(first_dim, initializer_list.size()), storage.begin());
			} else {
				for (auto& line: storage) {
					line = *initializer_list.begin();
				}
			}
		}

		// make convertible elements accepatable by 1d matrix intialization
		template<typename E>
		requires (dimensions.size() == 1uz && std::is_convertible_v<E, Element>)
		constexpr Matrix(std::initializer_list<E const> initializer_list) noexcept {
			if (initializer_list.size() > 1) {
				std::copy_n(initializer_list.begin(), std::min(first_dim, initializer_list.size()), storage.begin());
			} else {
				for (auto& line: storage) {
					line = *initializer_list.begin();
				}
			}
		}

		template<typename E>
		requires std::is_convertible_v<E, Element>
		constexpr Matrix(std::span<E const> initializer_list) noexcept
		{
			if (initializer_list.size() > 1) {
				std::copy_n(initializer_list.begin(), std::min(first_dim, initializer_list.size()), storage.begin());
			} else {
				for (auto& line: storage) {
					line = *initializer_list.begin();
				}
			}
		}

		template<typename U>
		requires std::is_convertible_v<U, T>
		explicit constexpr Matrix(U&& scalar) noexcept {
			if constexpr (dimensions.size() == 1) {
				storage.fill(scalar);
			} else if constexpr (dimensions.size() == 2) {
				auto constexpr diagonal_n = std::min(dimensions[0], dimensions[1]);
				for (auto i = 0; i < diagonal_n; i++) {
					storage[i][i] = std::forward<U>(scalar);
				}
			} else {
				for (auto& line: storage) {
					line = Element{std::forward<U>(scalar)};
				}
			}  
		};

		template<typename... Args>
		requires (true
			&& (std::is_convertible_v<Args, T> && ...)
			&& dimensions.size() > 1
			&& sizeof...(Args) <= std::min(*(dimensions.end() - 2), *(dimensions.end() - 1))
		)
		explicit constexpr Matrix(Args&&... args) noexcept {
			if constexpr (dimensions.size() > 2) {
				for (auto& line: storage) {
					line = {args...};
				}
			} else {
				[this, args...]<usize... idxs>(std::index_sequence<idxs...>) {
					((storage[idxs][idxs] = args), ...);
				}(std::make_index_sequence<sizeof...(Args)>{});
			}
		}

		template<typename U, typename... Args, usize rhs_first_dim>
		requires (true
			&& std::is_convertible_v<U, T>
			&& (std::is_convertible_v<Args, Element> && ...)
		)
		constexpr Matrix(Matrix<U, rhs_first_dim, rest_dims...> const& rhs, Args&&... rest) noexcept {
			*this = rhs;
			if constexpr (first_dim > rhs_first_dim) {
				[this, &rest...]<usize... idxs>(std::index_sequence<idxs...>) {
						((storage[rhs_first_dim + idxs] = std::forward<Args>(rest)), ...);
				}(std::make_index_sequence<std::min(sizeof...(rest), first_dim - rhs_first_dim)>{});
			}
		}

		template<typename U, typename... Args, usize rhs_first_dim>
		requires (true
			&& std::is_convertible_v<U, T>
			&& (std::is_convertible_v<Args, Element> && ...)
		)
		constexpr Matrix(Matrix<U, rhs_first_dim, rest_dims...>&& rhs, Args&&... rest) noexcept {
			*this = std::move(rhs);
			if constexpr (first_dim > rhs_first_dim) {
				[this, &rest...]<usize... idxs>(std::index_sequence<idxs...>) {
						((storage[rhs_first_dim + idxs] = std::forward<Args>(rest)), ...);
				}(std::make_index_sequence<std::min(sizeof...(rest), first_dim - rhs_first_dim)>{});
			}
		}

		template<usize rhs_first_dim0, usize rhs_first_dim1>
		constexpr Matrix(
			Matrix<T, rhs_first_dim0, rest_dims...> const& rhs0,
			Matrix<T, rhs_first_dim1, rest_dims...> const& rhs1
		) noexcept {
			*this = rhs0;
			if constexpr (first_dim > rhs_first_dim0) {
				std::copy_n(rhs1.storage.begin(), std::min(first_dim, rhs_first_dim1) - rhs_first_dim0, storage.begin() + rhs_first_dim0);
			}
		}

		template<usize rhs_first_dim0, usize rhs_first_dim1>
		constexpr Matrix(
			Matrix<T, rhs_first_dim0, rest_dims...>&& rhs0,
			Matrix<T, rhs_first_dim1, rest_dims...>&& rhs1
		) {
			*this = std::move(rhs0);
			if constexpr (first_dim > rhs_first_dim0) {
				std::move(rhs1.storage.begin(), rhs1.storage.begin() + (std::min(first_dim, rhs_first_dim1) - rhs_first_dim0), storage.begin() + rhs_first_dim0);
			}
		}

		template<typename U, usize rhs_first_dim, usize... rhs_rest_dims>
		requires true
			&& std::is_convertible_v<U, T>
			&& (sizeof...(rest_dims) == sizeof...(rhs_rest_dims))
		auto constexpr operator=(Matrix<U, rhs_first_dim, rhs_rest_dims...> const& rhs) noexcept -> Matrix& {
			std::copy_n(rhs.storage.begin(), std::min(first_dim, rhs_first_dim), storage.begin());
			return *this;
		}

		template<typename U, usize rhs_first_dim, usize... rhs_rest_dims>
		requires true
			&& std::is_convertible_v<U, T>
			&& (sizeof...(rest_dims) == sizeof...(rhs_rest_dims))
		auto constexpr operator=(Matrix<U, rhs_first_dim, rhs_rest_dims...>&& rhs) noexcept -> Matrix& {
			std::move(rhs.storage.begin(), rhs.storage.begin() + std::min(first_dim, rhs_first_dim), storage.begin());
			return *this;
		}

		auto constexpr operator[](usize idx) noexcept -> Element& {
			return storage[idx];
		}

		auto constexpr operator[](usize idx) const noexcept -> Element const& {
			return storage[idx];
		}

		template<
			usize... rhs_dims,
			usize l_n = dimensions.size(),
			usize r_n = sizeof...(rhs_dims),
			usize shorter_n = std::min(l_n, r_n),
			usize longer_n = std::max(l_n, r_n),
			usize higher_n = std::max(usize(0), longer_n - 2),
			std::array<usize, l_n> lds = dimensions,
			std::array<usize, r_n> rds = {rhs_dims...}
		>
		requires (true
			&& longer_n > 1
			&& (false
				|| i32(l_n) - i32(r_n) < 2
				|| i32(r_n) - i32(l_n) < 2
			) // clangd could not use math::abs
			&& []() noexcept -> bool {
				return std::equal(
					lds.begin(), lds.begin() + higher_n,
					rds.begin(), rds.begin() + higher_n
				);
			}()
			&& []() noexcept -> bool {
				return lds[higher_n + (l_n > 1 ? 1 : 0)] == rds[higher_n];
			}()
		)
		auto constexpr operator|(
			Matrix<T, rhs_dims...> const& rhs
		) const noexcept {
			using Product_Matrix = decltype([]<usize... dims>(std::index_sequence<dims...>) {
				return Matrix<T, (
					dims < higher_n ? lds[dims] : 
					dims == higher_n ? (l_n < r_n ? rds[higher_n + 1] : lds[higher_n]) : 
					rds[higher_n + 1]
				)...>{};
			}(std::make_index_sequence<shorter_n>{}));
			auto constexpr pds = Product_Matrix::dimensions;
			auto product = Product_Matrix{};

			if constexpr (higher_n > 0) {
				for (auto i = 0; i < first_dim; i++) {
					product[i] = storage[i] * rhs[i];
				}
			} else {
				if constexpr (l_n == 1 && r_n == 1) {
					
				} else if constexpr (l_n == 1) {
					for (auto i = 0; i < pds[0]; i++) {
						for (auto j = 0; j < lds[0]; j++) {
							product[i] += storage[j] * rhs[j][i];
						}
					}
				} else if constexpr (r_n == 1) {
					for (auto i = 0; i < pds[0]; i++) {
						for (auto j = 0; j < rds[0]; j++) {
							product[i] += storage[i][j] * rhs[j];
						}
					}
				} else {
					for (auto i = 0; i < pds[0]; i++) {
						for (auto k = 0; k < pds[1]; k++) {
							for (auto j = 0; j < lds[1]; j++) {
								product[i][k] += storage[i][j] * rhs[j][k];
							}
						}
					}
				}
			}

			return product;
		}

		auto constexpr operator+(Matrix const& rhs) const noexcept -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = storage[i] + rhs[i];
			}
			return result;
		}

		auto constexpr operator+=(Matrix const& rhs) noexcept -> Matrix& {
			*this = *this + rhs;
			return *this;
		}

		auto constexpr operator+(T const& rhs) const noexcept -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = storage[i] + rhs;
			}
			return result;
		}

		auto constexpr operator+=(T const& rhs) noexcept -> Matrix& {
			*this = *this + rhs;
			return *this;
		}

		auto constexpr operator+() noexcept -> Matrix {
			return *this;
		}

		auto constexpr operator-(Matrix const& rhs) const noexcept -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = storage[i] - rhs[i];
			}
			return result;
		}

		auto constexpr operator-=(Matrix const& rhs) noexcept -> Matrix& {
			*this = *this - rhs;
			return *this;
		}

		auto constexpr operator-(T const& rhs) const noexcept -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = storage[i] - rhs;
			}
			return result;
		}

		auto constexpr operator-=(T const& rhs) noexcept -> Matrix& {
			*this = *this - rhs;
			return *this;
		}

		auto constexpr operator-() const noexcept -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = -storage[i];
			}
			return result;
		}

		auto constexpr operator*(Matrix const& rhs) const noexcept -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = storage[i] * rhs[i];
			}
			return result;
		}

		auto constexpr operator*=(Matrix const& rhs) noexcept -> Matrix& {
			*this = *this * rhs;
			return *this;
		}

		auto constexpr operator*(T const& rhs) const noexcept -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = storage[i] * rhs;
			}
			return result;
		}

		auto constexpr operator*=(T const& rhs) noexcept -> Matrix& {
			*this = *this * rhs;
			return *this;
		}

		auto constexpr operator/(Matrix const& rhs) const noexcept -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = math::guarded_div(storage[i], rhs[i]);
			}
			return result;
		}

		auto constexpr operator/=(Matrix const& rhs) noexcept -> Matrix& {
			*this = *this / rhs;
			return *this;
		}

		auto constexpr operator/(T const& rhs) const noexcept -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = math::guarded_div(storage[i], rhs);
			}
			return result;
		}

		auto constexpr operator/=(T const& rhs) noexcept -> Matrix& {
			*this = *this / rhs;
			return *this;
		}

		auto constexpr operator<=>(Matrix const& rhs) const = default;

		operator std::array<Element, first_dim>&() {
			return storage;
		};

		operator std::array<Element, first_dim> const&() const {
			return storage;
		};

		auto constexpr data() -> mut<T> {
			return mut<T>(&storage);
		};

		template<usize idx>
		auto constexpr get() const noexcept -> Element const& {
			static_assert(idx < first_dim, "index out of bounds");
			return storage[idx];
		}

		template<usize idx>
		auto constexpr get() noexcept -> Element& {
			static_assert(idx < first_dim, "index out of bounds");
			return storage[idx];
		}

	private:
		std::array<Element, first_dim> storage{};

		template<typename U, usize... dims>
		friend struct Matrix;
	};

	template<typename T, usize... dims>
	auto constexpr operator+(T const& lhs, Matrix<T, dims...> const& rhs) noexcept -> Matrix<T, dims...> {
		return rhs + lhs;
	}

	template<typename T, usize... dims>
	auto constexpr operator-(T const& lhs, Matrix<T, dims...> const& rhs) noexcept -> Matrix<T, dims...> {
		return -rhs + lhs;
	}

	template<typename T, usize... dims>
	auto constexpr operator*(T const& lhs, Matrix<T, dims...> const& rhs) noexcept -> Matrix<T, dims...> {
		return rhs * lhs;
	}

	template<typename T, usize... dims>
	auto constexpr operator/(T const& lhs, Matrix<T, dims...> const& rhs) noexcept -> Matrix<T, dims...> {
		return Matrix<T, dims...>{lhs} / rhs;
	}

	template<usize idx, typename T, usize first_dim, usize... rest_dims>
	auto constexpr get(Matrix<T, first_dim, rest_dims...> const& m) 
		noexcept -> typename Matrix<T, first_dim, rest_dims...>::Element const& {
		return m.template get<idx>();
	}

	template<usize idx, typename T, usize first_dim, usize... rest_dims>
	auto constexpr get(Matrix<T, first_dim, rest_dims...>& m) 
		noexcept -> typename Matrix<T, first_dim, rest_dims...>::Element& {
		return m.template get<idx>();
	}

	template<typename T, usize h, usize w>
	auto constexpr transpose(Matrix<T, h, w> const& m) noexcept -> Matrix<T, w, h> {
		auto result = Matrix<T, w, h>{};
		for (auto i = 0; i < w; i++) {
			for (auto j = 0; j < h; j++) {
				result[i][j] = m[j][i];
			}
		}
		return result;
	}

	template<typename T, usize n>
	requires std::floating_point<T>
	auto constexpr determinant(Matrix<T, n, n> const& m) noexcept -> T {
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
			T det = T{1};
			
			for (usize i = 0; i < n; i++) {
				usize pivot_row = i;
				T max_val = math::abs(u[i][i]);
				
				for (usize j = i + 1; j < n; j++) {
					if (auto curr_val = math::abs(u[j][i]); curr_val > max_val) {
						max_val = curr_val;
						pivot_row = j;
					}
				}
				
				if (max_val < std::numeric_limits<T>::epsilon()) {
					return T{0};
				}
				if (pivot_row != i) {
					std::swap(u[i], u[pivot_row]);
					det = -det;
				}
				det *= u[i][i];

				for (usize j = i + 1; j < n; j++) {
					T factor = u[j][i] / u[i][i];
					for (usize k = i + 1; k < n; k++) {
						u[j][k] -= factor * u[i][k];
					}
				}
			}
			
			return det;
		}
	}

	template<typename T, usize h>
	requires std::floating_point<T>
	auto constexpr inverse(Matrix<T, h, h> const& m) noexcept -> Matrix<T, h, h> {
		auto augmented = Matrix<T, h, h * 2>{};
		for (usize i = 0; i < h; i++) {
			for (usize j = 0; j < h; j++) {
				augmented[i][j] = m[i][j];
			}
		}
		for (usize i = 0; i < h; i++) {
			augmented[i][h + i] = T(1);
		}

		// Gaussian-Jordan
		for (usize i = 0; i < h; i++) {
			auto pivot_row = i;
			auto max_val = math::abs(augmented[i][i]);
			
			for (usize j = i + 1; j < h; j++) {
				if (auto curr_val = math::abs(augmented[j][i]); curr_val > max_val) {
					max_val = curr_val;
					pivot_row = j;
				}
			}

			if (pivot_row != i) {
				std::swap(augmented[i], augmented[pivot_row]);
			}

			auto pivot = augmented[i][i];
			augmented[i] /= pivot;

			for (usize j = 0; j < h; j++) {
				if (j != i) {
					auto factor = augmented[j][i];
					augmented[j] -= factor * augmented[i];
				}
			}
		}

		auto result = Matrix<T, h, h>{};
		for (usize i = 0; i < h; i++) {
			for (usize j = 0; j < h; j++) {
				result[i][j] = augmented[i][h + j];
			}
		}
		return result;
	}

	template<typename T, usize h, usize w>
	requires std::floating_point<T>
	auto constexpr least_squares(Matrix<T, h, w> const& a, Matrix<T, h> const& b) noexcept -> Matrix<T, w> {
		auto a_t = math::transpose(a);
		return math::inverse(a_t | a) | (a_t | b);
	}

	template<typename T, usize n>
	requires std::floating_point<T>
	auto constexpr cramer(
		Matrix<T, n, n> const& a, 
		Matrix<T, n> const& b
	) noexcept -> std::optional<Matrix<T, n>> {
		T det_a = determinant(a);
		if (math::abs(det_a) < epsilon<T>) {
			return {};
		}

		auto result = Matrix<T, n>{};
		for (auto i = 0uz; i < n; i++) {
			auto a_i = a;
			for (auto j = 0uz; j < n; j++) {
				a_i[j][i] = b[j];
			}
			result[i] = determinant(a_i) / det_a;
		}
		
		return result;
	}

	template<typename T, usize n, usize m>
	requires std::floating_point<T>
	auto constexpr cramer(
		Matrix<T, n, n> const& a, 
		Matrix<T, n, m> const& b,
		bool check_degeneration = false
	) noexcept -> std::optional<Matrix<T, n, m>> {
		T det_a = determinant(a);
		if (check_degeneration && math::abs(det_a) < 1e-12f) {
			return {};
		}

		auto result = Matrix<T, n, m>{};
		if constexpr (n == 2) {
			result[0] = (a[1][1] * b[0] - a[0][1] * b[1]) / det_a;
			result[1] = (a[1][0] * b[0] - a[0][0] * b[1]) / det_a;
		} else {
			for (auto i = 0uz; i < n; i++) {
				auto a_i = a;
				for (auto j = 0uz; j < m; j++) {
					for (auto k = 0uz; k < n; k++) {
						a_i[k][i] = b[k][j];
					}
					result[i][j] = determinant(a_i) / det_a;
				}
			}
		}
		
		return result;
	}
}

namespace std {
	using namespace mtt;

	template<typename T, usize first_dim, usize... rest_dims>
	struct tuple_size<math::Matrix<T, first_dim, rest_dims...>> 
		: std::integral_constant<size_t, first_dim> {};

	template<size_t I, typename T, usize first_dim, usize... rest_dims>
	struct tuple_element<I, math::Matrix<T, first_dim, rest_dims...>> {
		using type = typename math::Matrix<T, first_dim, rest_dims...>::Element;
	};
}
