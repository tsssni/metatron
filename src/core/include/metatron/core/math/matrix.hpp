#pragma once
#include <algorithm>
#include <compare>
#include <functional>
#include <initializer_list>
#include <array>
#include <cmath>
#include <cassert>
#include <cstdio>
#include <type_traits>
#include <utility>

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

		Matrix(std::initializer_list<Element> initializer_list) {
			assert(initializer_list.size() <= first_dim);
			if (initializer_list.size() > 1) {
				std::copy(initializer_list.begin(), initializer_list.end(), data.begin());
			} else {
				for (auto& line: data) {
					line = initializer_list.size() == 1 ? *initializer_list.begin() : Element{};
				}
			}
		}

		explicit Matrix(T&& scalar) {
			if constexpr (dimensions.size() == 1) {
				data.fill(scalar);
			} else if constexpr (dimensions.size() == 2) {
				auto constexpr diagonal_n = std::min(dimensions[0], dimensions[1]);
				for (auto i = 0; i < diagonal_n; i++) {
					data[i][i] = std::forward<T>(scalar);
				}
			} else {
				for (auto& line: data) {
					line = Element{std::forward<T>(scalar)};
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

		template<usize rhs_first_dim, usize... rhs_rest_dims>
		Matrix(Matrix<T, rhs_first_dim, rhs_rest_dims...> const& rhs) {
			*this = rhs;
		}

		template<usize rhs_first_dim, usize... rhs_rest_dims>
		requires (sizeof...(rest_dims) == sizeof...(rhs_rest_dims))
			&& (rhs_first_dim <= first_dim)
			&& (... && (rhs_rest_dims <= rest_dims))
		auto operator=(Matrix<T, rhs_first_dim, rhs_rest_dims...> const& rhs) -> Matrix& {
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
				if constexpr (l_n == 1) {
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
			return operate([](Element const& a, Element const& b) { return a + b; }, rhs);
		}

		auto operator+=(Matrix const& rhs) -> Matrix& {
			*this = *this + rhs;
			return *this;
		}

		auto operator+(T&& rhs) const -> Matrix {
			return operate([](Element const& a, T&& b) { return a + std::forward<T>(b); }, std::forward<T>(rhs));
		}

		auto operator+=(T&& rhs) -> Matrix& {
			*this = *this + std::forward<T>(rhs);
			return *this;
		}

		auto operator+() -> Matrix {
			return *this;
		}

		auto operator-(Matrix const& rhs) const -> Matrix {
			return operate([](Element const& a, Element const& b) { return a - b; }, rhs);
		}

		auto operator-=(Matrix const& rhs) -> Matrix& {
			*this = *this - rhs;
			return *this;
		}

		auto operator-(T&& rhs) const -> Matrix {
			return operate([](Element const& a, T&& b) { return a - std::forward<T>(b); }, std::forward<T>(rhs));
		}

		auto operator-=(T&& rhs) -> Matrix& {
			*this = *this - std::forward<T>(rhs);
			return *this;
		}

		auto operator-() const -> Matrix {
			return operate([](Element const& a, Element const& b) { return -a; }, *this);
		}

		auto operator*(T&& rhs) const -> Matrix {
			return operate([](Element const& a, T&& b) { return a * std::forward<T>(b); }, std::forward<T>(rhs));
		}

		auto operator*=(T&& rhs) -> Matrix& {
			*this = *this * std::forward<T>(rhs);
			return *this;
		}

		auto operator/(Matrix const& rhs) const -> Matrix {
			return operate([](Element const& a, Element const& b) { return a / b; }, rhs);
		}

		auto operator/=(Matrix const& rhs) -> Matrix& {
			*this = *this / rhs;
			return *this;
		}

		auto operator/(T&& rhs) const -> Matrix {
			return operate([](Element const& a, T&& b) { return a / std::forward<T>(b); }, std::forward<T>(rhs));
		}

		auto operator/=(T&& rhs) -> Matrix& {
			*this = *this / std::forward<T>(rhs);
			return *this;
		}

		auto operator<=>(Matrix const& rhs) const = default;

	private:
		auto operate(std::function<Element(Element const& a, Element const& b)> f, Matrix const& rhs) const -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = f(data[i], rhs[i]);
			}
			return result;
		}

		auto operate(std::function<Element(Element const& a, T&& b)> f, T&& rhs) const -> Matrix {
			auto result = Matrix{};
			for (auto i = 0; i < first_dim; i++) {
				result[i] = f(data[i], std::forward<T>(rhs));
			}
			return result;
		}

		std::array<Element, first_dim> data{};

		template<typename U, usize... dims>
		friend struct Matrix;
	};

	using Matrix2 = Matrix<f32, 2, 2>;
	using Matrix3 = Matrix<f32, 3, 3>;
	using Matrix4 = Matrix<f32, 4, 4>;

	template<typename T, metatron::usize... dims>
	auto inline operator+(T&& lhs, metatron::math::Matrix<T, dims...> const& rhs) {
		return rhs + std::forward<T>(lhs);
	}

	template<typename T, metatron::usize... dims>
	auto inline operator-(T&& lhs, metatron::math::Matrix<T, dims...> const& rhs) {
		return -rhs + std::forward<T>(lhs);
	}

	template<typename T, metatron::usize... dims>
	auto inline operator*(T&& lhs, metatron::math::Matrix<T, dims...> const& rhs) {
		return rhs * std::forward<T>(lhs);
	}
}
