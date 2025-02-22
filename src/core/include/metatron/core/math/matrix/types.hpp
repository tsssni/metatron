#pragma once
#include <algorithm>
#include <initializer_list>
#include <array>
#include <cmath>
#include <cassert>
#include <cstdio>

namespace metatron::math {
	template<typename T, usize first_dim, usize... rest_dims>
	requires (first_dim > 0 && (... && (rest_dims > 0)))
	struct Matrix {
		using Element = Matrix<T, rest_dims...>;
		auto static constexpr dimensions = std::array<usize, 1 + sizeof...(rest_dims)>{first_dim, rest_dims...};

		Matrix() = default;

		Matrix(std::initializer_list<Element> initializer_list) {
			assert(initializer_list.size() <= first_dim);
			std::copy(initializer_list.begin(), initializer_list.end(), data.begin());
		}

		Matrix(T const& scalar) {
			*this = scalar;
		};

		auto operator=(T const& scalar) -> Matrix& {
			if constexpr (dimensions.size() > 2) {
				for (auto& line: data) {
					line = scalar;
				}
			} else {
				auto constexpr diagonal_size = std::min(dimensions[0], dimensions[1]);
				for (auto i = 0; i < diagonal_size; i++) {
					data[i][i] = scalar;
				}
			}
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
	};

	template<typename T, usize dim>
	requires (dim > 0)
	struct Matrix<T, dim> {
		using Element = T;

		Matrix() = default;

		Matrix(std::initializer_list<Element> initializer_list) {
			assert(initializer_list.size() <= dim);
			std::copy(initializer_list.begin(), initializer_list.end(), data.begin());
		}

		Matrix(Element const& element) {
			data.fill(element);
		}

		auto operator[](usize idx) -> T& {
			return data[idx];
		}

		auto operator[](usize idx) const -> T const& {
			return data[idx];
		}

	private:
		std::array<Element, dim> data{};
	};

	using Matrix2 = Matrix<f32, 2, 2>;
	using Matrix3 = Matrix<f32, 3, 3>;
	using Matrix4 = Matrix<f32, 4, 4>;
}
