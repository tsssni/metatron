#pragma once
#include <algorithm>
#include <array>
#include <cassert>

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

		Matrix(std::initializer_list<Element> initializer_list)
		{
			assert(initializer_list.size() <= first_dim);
			if (initializer_list.size() > 1) {
				std::copy(initializer_list.begin(), initializer_list.end(), data.begin());
			} else {
				for (auto& line: data) {
					line = initializer_list.size() == 1 ? *initializer_list.begin() : Element{};
				}
			}
		}

		template<typename U>
		requires std::is_convertible_v<U, T>
		explicit Matrix(U&& scalar) {
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
			&& ([]() -> bool {
				if constexpr (dimensions.size() == 1) {
					return sizeof...(Args) == first_dim;
				} else {
					auto n = dimensions.size();
					return sizeof...(Args) == std::min(dimensions[n - 2], dimensions[n - 1]);
				}
			}())
		)
		explicit Matrix(Args&&... args) {
			if constexpr (dimensions.size() > 2) {
				for (auto& line: data) {
					line = {args...};
				}
			} else if constexpr(dimensions.size() == 1) {
				data.fill(args...);
			} else {
				auto constexpr n = dimensions.size();
				auto constexpr diagonal_n = std::min(dimensions[n - 2], dimensions[n - 1]);
				[this, args...]<usize... idxs>(std::index_sequence<idxs...>) {
					((data[idxs][idxs] = args), ...);
				}(std::make_index_sequence<diagonal_n>{});
			}
		}

		template<typename U, usize rhs_first_dim, usize... rhs_rest_dims>
		requires std::is_convertible_v<U, T>
		Matrix(Matrix<U, rhs_first_dim, rhs_rest_dims...> const& rhs) {
			*this = rhs;
		}

		template<typename U, usize rhs_first_dim, usize... rhs_rest_dims>
		requires true
			&& std::is_convertible_v<U, T>
			&& (sizeof...(rest_dims) == sizeof...(rhs_rest_dims))
			&& (rhs_first_dim <= first_dim)
			&& (... && (rhs_rest_dims <= rest_dims))
		auto operator=(Matrix<U, rhs_first_dim, rhs_rest_dims...> const& rhs) -> Matrix& {
			std::copy_n(rhs.data.begin(), rhs_first_dim, data.begin());
			return *this;
		}

		auto operator[](usize idx) -> Element& {
			return data[idx];
		}

		auto operator[](usize idx) const -> Element const& {
			return data[idx];
		}

		template<
			usize... rhs_dims,
			usize l_n = dimensions.size(),
			usize r_n = sizeof...(rhs_dims),
			usize shorter_n = std::min(l_n, r_n),
			usize longer_n = std::max(l_n, r_n),
			usize higher_n = std::max(usize(0), longer_n - (longer_n == 1 ? 1 : 2)),
			std::array<usize, l_n> lds = dimensions,
			std::array<usize, r_n> rds = {rhs_dims...}
		>
		requires (true
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
		auto operator*(
			Matrix<T, rhs_dims...> const& rhs
		) {
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

		auto operator+(Matrix const& rhs) const -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = data[i] + rhs[i];
			}
			return result;
		}

		auto operator+=(Matrix const& rhs) -> Matrix& {
			*this = *this + rhs;
			return *this;
		}

		auto operator+(T const& rhs) const -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = data[i] + rhs;
			}
			return result;
		}

		auto operator+=(T const& rhs) -> Matrix& {
			*this = *this + rhs;
			return *this;
		}

		auto operator+() -> Matrix {
			return *this;
		}

		auto operator-(Matrix const& rhs) const -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = data[i] - rhs[i];
			}
			return result;
		}

		auto operator-=(Matrix const& rhs) -> Matrix& {
			*this = *this - rhs;
			return *this;
		}

		auto operator-(T const& rhs) const -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = data[i] - rhs;
			}
			return result;
		}

		auto operator-=(T const& rhs) -> Matrix& {
			*this = *this - rhs;
			return *this;
		}

		auto operator-() const -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = -data[i];
			}
			return result;
		}

		auto operator*(T const& rhs) const -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = data[i] * rhs;
			}
			return result;
		}

		auto operator*=(T const& rhs) -> Matrix& {
			*this = *this * rhs;
			return *this;
		}

		auto operator/(Matrix const& rhs) const -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = data[i] / rhs[i];
			}
			return result;
		}

		auto operator/=(Matrix const& rhs) -> Matrix& {
			*this = *this / rhs;
			return *this;
		}

		auto operator/(T const& rhs) const -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = data[i] / rhs;
			}
			return result;
		}

		auto operator/=(T const& rhs) -> Matrix& {
			*this = *this / rhs;
			return *this;
		}

		auto operator<=>(Matrix const& rhs) const = default;

	private:
		std::array<Element, first_dim> data;

		template<typename U, usize... dims>
		friend struct Matrix;
	};

	template<typename T, usize... dims>
	auto inline operator+(T const& lhs, Matrix<T, dims...> const& rhs) -> Matrix<T, dims...> {
		return rhs + lhs;
	}

	template<typename T, usize... dims>
	auto inline operator-(T const& lhs, Matrix<T, dims...> const& rhs) -> Matrix<T, dims...> {
		return -rhs + lhs;
	}

	template<typename T, usize... dims>
	auto inline operator*(T const& lhs, Matrix<T, dims...> const& rhs) -> Matrix<T, dims...> {
		return rhs * lhs;
	}

	template<typename T, usize h, usize w>
	auto inline transpose(Matrix<T, h, w> const& m) -> Matrix<T, w, h> {
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
	auto inline inverse(Matrix<T, h, h> const& m) -> Matrix<T, h, h> {
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

			if (max_val < std::numeric_limits<T>::epsilon()) {
				assert("matrix is not invertible");
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
}
