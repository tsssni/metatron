#pragma once
#include <algorithm>
#include <initializer_list>
#include <array>
#include <cmath>
#include <cassert>
#include <cstdio>
#include <type_traits>

namespace metatron::math {
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
				if constexpr (dimensions.size() != 2) {
					for (auto& line: data) {
						line = initializer_list.size() == 1 ? *initializer_list.begin() : Element{};
					}
				} else {
					// to make Matrix<T, N, N>{T{}} same as Matrix<T, N, N>(T{})
					*this = (*initializer_list.begin())[0];
				}
			}
		}

		Matrix(T const& scalar) {
			*this = scalar;
		};

		template<usize rhs_first_dim, usize... rhs_rest_dims>
		requires (sizeof...(rest_dims) == sizeof...(rhs_rest_dims))
			&& (rhs_first_dim <= first_dim)
			&& (... && (rhs_rest_dims <= rest_dims))
		Matrix(Matrix<T, rhs_first_dim, rhs_rest_dims...> const& rhs) {
			*this = rhs;
		}

		auto operator=(T const& scalar) -> Matrix& {
			if constexpr (dimensions.size() > 2) {
				for (auto& line: data) {
					line = scalar;
				}
			} else if constexpr (dimensions.size() == 2) {
				auto constexpr diagonal_size = std::min(dimensions[0], dimensions[1]);
				for (auto i = 0; i < diagonal_size; i++) {
					data[i][i] = scalar;
				}
			} else if constexpr (dimensions.size() == 1) {
				data.fill(scalar);
			}
			return *this;
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

	private:
		std::array<Element, first_dim> data{};

		template<typename U, usize... dims>
		friend struct Matrix;
	};

	using Matrix2 = Matrix<f32, 2, 2>;
	using Matrix3 = Matrix<f32, 3, 3>;
	using Matrix4 = Matrix<f32, 4, 4>;
}
