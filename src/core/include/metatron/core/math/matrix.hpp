#pragma once
#include <metatron/core/math/constant.hpp>
#include <algorithm>
#include <array>
#include <span>
#include <cassert>
#include <cmath>

namespace metatron::math {
	// forward declaration to support the declaration of 0d matrix
	template<typename T, usize... dims>
	struct Matrix;

	template<typename T, usize first_dim, usize... rest_dims>
	requires (first_dim > 0 && (... && (rest_dims > 0)))
	struct Matrix<T, first_dim, rest_dims...> {
		using Element = std::conditional_t<sizeof...(rest_dims) == 0, T, Matrix<T, rest_dims...>>;
		auto static constexpr dimensions = std::array<usize, 1 + sizeof...(rest_dims)>{first_dim, rest_dims...};

		Matrix() = default;

		// directly use Element instead of template type to enable auto inference
		constexpr Matrix(std::initializer_list<Element const> initializer_list) {
			if (initializer_list.size() > 1) {
				std::copy_n(initializer_list.begin(), std::min(first_dim, initializer_list.size()), data.begin());
			} else {
				for (auto& line: data) {
					line = initializer_list.size() == 1 ? *initializer_list.begin() : Element{};
				}
			}
		}

		// make convertible elements accepatable by 1d matrix intialization
		template<typename E>
		requires (dimensions.size() == 1uz && std::is_convertible_v<E, Element>)
		constexpr Matrix(std::initializer_list<E const> initializer_list) {
			if (initializer_list.size() > 1) {
				std::copy_n(initializer_list.begin(), std::min(first_dim, initializer_list.size()), data.begin());
			} else {
				for (auto& line: data) {
					line = initializer_list.size() == 1 ? *initializer_list.begin() : Element{};
				}
			}
		}

		template<typename E>
		requires std::is_convertible_v<E, Element>
		constexpr Matrix(std::span<E const> initializer_list)
		{
			if (initializer_list.size() > 1) {
				std::copy_n(initializer_list.begin(), std::min(first_dim, initializer_list.size()), data.begin());
			} else {
				for (auto& line: data) {
					line = *initializer_list.begin();
				}
			}
		}

		template<typename U>
		requires std::is_convertible_v<U, T>
		explicit constexpr Matrix(U&& scalar) {
			if constexpr (dimensions.size() == 1) {
				data.fill(scalar);
			} else if constexpr (dimensions.size() == 2) {
				auto constexpr diagonal_n = std::min(dimensions[0], dimensions[1]);
				for (auto i = 0; i < diagonal_n; i++) {
					data[i][i] = std::forward<U>(scalar);
				}
			} else {
				for (auto& line: data) {
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
		explicit constexpr Matrix(Args&&... args) {
			if constexpr (dimensions.size() > 2) {
				for (auto& line: data) {
					line = {args...};
				}
			} else {
				[this, args...]<usize... idxs>(std::index_sequence<idxs...>) {
					((data[idxs][idxs] = args), ...);
				}(std::make_index_sequence<sizeof...(Args)>{});
			}
		}

		template<typename U, typename... Args, usize rhs_first_dim>
		requires (true
			&& std::is_convertible_v<U, T>
			&& (std::is_convertible_v<Args, Element> && ...)
		)
		constexpr Matrix(Matrix<U, rhs_first_dim, rest_dims...> const& rhs, Args&&... rest) {
			*this = rhs;
			if constexpr (first_dim > rhs_first_dim) {
				[this, rest...]<usize... idxs>(std::index_sequence<idxs...>) {
						((data[rhs_first_dim + idxs] = rest), ...);
				}(std::make_index_sequence<std::min(sizeof...(rest), first_dim - rhs_first_dim)>{});
			}

		}

		template<usize rhs_first_dim0, usize rhs_first_dim1>
		constexpr Matrix(
			Matrix<T, rhs_first_dim0, rest_dims...> const& rhs0,
			Matrix<T, rhs_first_dim1, rest_dims...> const& rhs1
		) {
			*this = rhs0;
			if constexpr (first_dim > rhs_first_dim0) {
				std::copy_n(rhs1.data.begin(), std::min(first_dim, rhs_first_dim1) - rhs_first_dim0, data.begin() + rhs_first_dim0);
			}
		}

		template<typename U, usize rhs_first_dim, usize... rhs_rest_dims>
		requires true
			&& std::is_convertible_v<U, T>
			&& (sizeof...(rest_dims) == sizeof...(rhs_rest_dims))
		auto constexpr operator=(Matrix<U, rhs_first_dim, rhs_rest_dims...> const& rhs) -> Matrix& {
			std::copy_n(rhs.data.begin(), std::min(first_dim, rhs_first_dim), data.begin());
			return *this;
		}

		auto constexpr operator[](usize idx) -> Element& {
			return data[idx];
		}

		auto constexpr operator[](usize idx) const -> Element const& {
			return data[idx];
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
			) // clangd could not use std::abs
			&& []() -> bool {
				return std::equal(
					lds.begin(), lds.begin() + higher_n,
					rds.begin(), rds.begin() + higher_n
				);
			}()
			&& []() -> bool {
				return lds[higher_n + (l_n > 1 ? 1 : 0)] == rds[higher_n];
			}()
		)
		auto constexpr operator|(
			Matrix<T, rhs_dims...> const& rhs
		) const {
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
					product[i] = data[i] * rhs[i];
				}
			} else {
				if constexpr (l_n == 1 && r_n == 1) {
					
				} else if constexpr (l_n == 1) {
					for (auto i = 0; i < pds[0]; i++) {
						for (auto j = 0; j < lds[0]; j++) {
							product[i] += data[j] * rhs[j][i];
						}
					}
				} else if constexpr (r_n == 1) {
					for (auto i = 0; i < pds[0]; i++) {
						for (auto j = 0; j < rds[0]; j++) {
							product[i] += data[i][j] * rhs[j];
						}
					}
				} else {
					for (auto i = 0; i < pds[0]; i++) {
						for (auto k = 0; k < pds[1]; k++) {
							for (auto j = 0; j < lds[1]; j++) {
								product[i][k] += data[i][j] * rhs[j][k];
							}
						}
					}
				}
			}

			return product;
		}

		auto constexpr operator+(Matrix const& rhs) const -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = data[i] + rhs[i];
			}
			return result;
		}

		auto constexpr operator+=(Matrix const& rhs) -> Matrix& {
			*this = *this + rhs;
			return *this;
		}

		auto constexpr operator+(T const& rhs) const -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = data[i] + rhs;
			}
			return result;
		}

		auto constexpr operator+=(T const& rhs) -> Matrix& {
			*this = *this + rhs;
			return *this;
		}

		auto constexpr operator+() -> Matrix {
			return *this;
		}

		auto constexpr operator-(Matrix const& rhs) const -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = data[i] - rhs[i];
			}
			return result;
		}

		auto constexpr operator-=(Matrix const& rhs) -> Matrix& {
			*this = *this - rhs;
			return *this;
		}

		auto constexpr operator-(T const& rhs) const -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = data[i] - rhs;
			}
			return result;
		}

		auto constexpr operator-=(T const& rhs) -> Matrix& {
			*this = *this - rhs;
			return *this;
		}

		auto constexpr operator-() const -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = -data[i];
			}
			return result;
		}

		auto constexpr operator*(Matrix const& rhs) const -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = data[i] * rhs[i];
			}
			return result;
		}

		auto constexpr operator*=(Matrix const& rhs) -> Matrix& {
			*this = *this * rhs;
			return *this;
		}

		auto constexpr operator*(T const& rhs) const -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = data[i] * rhs;
			}
			return result;
		}

		auto constexpr operator*=(T const& rhs) -> Matrix& {
			*this = *this * rhs;
			return *this;
		}

		auto constexpr operator/(Matrix const& rhs) const -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = data[i] / rhs[i];
			}
			return result;
		}

		auto constexpr operator/=(Matrix const& rhs) -> Matrix& {
			*this = *this / rhs;
			return *this;
		}

		auto constexpr operator/(T const& rhs) const -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = data[i] / rhs;
			}
			return result;
		}

		auto constexpr operator/=(T const& rhs) -> Matrix& {
			*this = *this / rhs;
			return *this;
		}

		auto constexpr operator<=>(Matrix const& rhs) const = default;

	private:
		std::array<Element, first_dim> data{};

		template<typename U, usize... dims>
		friend struct Matrix;
	};

	template<typename T, usize... dims>
	auto inline constexpr operator+(T const& lhs, Matrix<T, dims...> const& rhs) -> Matrix<T, dims...> {
		return rhs + lhs;
	}

	template<typename T, usize... dims>
	auto inline constexpr operator-(T const& lhs, Matrix<T, dims...> const& rhs) -> Matrix<T, dims...> {
		return -rhs + lhs;
	}

	template<typename T, usize... dims>
	auto inline constexpr operator*(T const& lhs, Matrix<T, dims...> const& rhs) -> Matrix<T, dims...> {
		return rhs * lhs;
	}

	template<typename T, usize h, usize w>
	auto inline constexpr transpose(Matrix<T, h, w> const& m) -> Matrix<T, w, h> {
		auto result = Matrix<T, w, h>{};
		for (auto i = 0; i < w; i++) {
			for (auto j = 0; j < h; j++) {
				result[i][j] = m[j][i];
			}
		}
		return result;
	}

	template<typename T, usize h>
	requires std::floating_point<T>
	auto inline constexpr inverse(Matrix<T, h, h> const& m) -> Matrix<T, h, h> {
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
			auto max_row = i;
			auto max_val = std::abs(augmented[i][i]);
			
			for (usize j = i + 1; j < h; j++) {
				if (auto curr_val = std::abs(augmented[j][i]); curr_val > max_val) {
					max_val = curr_val;
					max_row = j;
				}
			}

			if (max_row != i) {
				std::swap(augmented[i], augmented[max_row]);
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
	auto inline constexpr least_squares(Matrix<T, h, w> const& a, Matrix<T, h> const& b) -> Matrix<T, w> {
		auto a_t = math::transpose(a);
		return math::inverse(a_t | a) | (a_t | b);
	}
}
