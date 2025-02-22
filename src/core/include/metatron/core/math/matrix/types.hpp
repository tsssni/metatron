#pragma once
#include <algorithm>
#include <concepts>
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

		explicit Matrix(T const& scalar) {
			if constexpr (dimensions.size() == 1) {
				data.fill(scalar);
			} else if constexpr (dimensions.size() == 2) {
				auto constexpr diagonal_n = std::min(dimensions[0], dimensions[1]);
				for (auto i = 0; i < diagonal_n; i++) {
					data[i][i] = scalar;
				}
			} else {
				for (auto& line: data) {
					line = scalar;
				}
			}  
		};

		template<typename... Args>
		requires (true
			&& (std::same_as<std::remove_reference_t<Args>, T> && ...)
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

	private:
		std::array<Element, first_dim> data{};

		template<typename U, usize... dims>
		friend struct Matrix;
	};

	using Matrix2 = Matrix<f32, 2, 2>;
	using Matrix3 = Matrix<f32, 3, 3>;
	using Matrix4 = Matrix<f32, 4, 4>;
}
