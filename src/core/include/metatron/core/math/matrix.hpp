#pragma once
#include <algorithm>
#include <initializer_list>
#include <array>
#include <cmath>

namespace metatron::math {
	template<typename T, usize first_dim, usize... rest_dims>
	struct Matrix {
		auto static constexpr dimensions =
			std::array<usize, 1 + sizeof...(rest_dims)>{first_dim, rest_dims...};
		using Line_Type = Matrix<T, rest_dims...>;

		Matrix() = default;

		Matrix(std::initializer_list<Line_Type> initializer_list) {
			static_assert(initializer_list.size() <= first_dim, "matrix initializer too long");
			std::copy(initializer_list.begin(), initializer_list.end(), data.begin());
		}

		Matrix(std::initializer_list<typename Line_Type::Line_Type> line) {
			if constexpr (dimensions.size() > 2) {
				data.fill(Line_Type{line});
			} else if (line.size() != 1) {
				data.fill(Line_Type{line});
			} else {
				// fill diagonal of 2D matrix
				auto constexpr diagonal_size = std::min(dimensions[0], dimensions[1]);
				for (auto i = 0; i < diagonal_size; i++) {
					data[i][i] = *line.begin();
				}
			}
		}

		auto operator[](usize idx) -> Line_Type& {
			return data[idx];
		}

	private:
		std::array<Line_Type, first_dim> data = {};
	};

	template<typename T, usize size>
	struct Matrix<T, size> {
		using Line_Type = T;

		Matrix() = default;

		Matrix(std::initializer_list<Line_Type> initializer_list) {
			std::copy(initializer_list.begin(), initializer_list.end(), data.begin());
		}

		Matrix(Line_Type const& element) {
			data.fill(element);
		}

		auto operator[](usize idx) -> T& {
			return data[idx];
		}

	private:
		std::array<Line_Type, size> data;
	};

	using Matrix2 = Matrix<f32, 2, 2>;
	using Matrix3 = Matrix<f32, 3, 3>;
	using Matrix4 = Matrix<f32, 4, 4>;
}
