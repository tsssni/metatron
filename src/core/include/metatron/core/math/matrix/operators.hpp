#pragma once
#include <metatron/core/math/matrix/types.hpp>

namespace metatron::math {
	namespace {
		template<typename T>
		struct Matrix_Multiply_Impl final {
			template<
				usize first_dim,
				usize... lhs_rest_dims,
				usize... rhs_rest_dims,
				usize... product_rest_dims
			>
			static void multiply(
				Matrix<T, first_dim, lhs_rest_dims...> const& lhs,
				Matrix<T, first_dim, rhs_rest_dims...> const& rhs,
				Matrix<T, first_dim, product_rest_dims...>& product
			) {
				if constexpr (sizeof...(product_rest_dims) > 2) {
					for (auto i = 0; i < first_dim; i++) {
						apply(lhs[i], rhs[i], product[i]);
					}
				} else {
					auto constexpr lhs_last_dims = std::array<usize, 2>{lhs_rest_dims...};
					auto constexpr rhs_last_dims = std::array<usize, 2>{rhs_rest_dims...};
					auto constexpr dim_i = *(lhs_last_dims.end() - 2);
					auto constexpr dim_j = *(lhs_last_dims.end() - 1);
					auto constexpr dim_k = *(rhs_last_dims.end() - 1);

					for (auto high_dim = 0; high_dim < first_dim; high_dim++) {
						for (auto i = 0; i < dim_i; i++) {
							for (auto k = 0; k < dim_k; k++) {
								for (auto j = 0; j < dim_j; j++) {
									product[high_dim][i][k] += lhs[high_dim][i][j] * rhs[high_dim][j][k];
								}
							}
						}
					}
				}
			}
		};
	}

	template<typename T, usize... lhs_dims, usize... rhs_dims>
	requires (true
		&& sizeof...(lhs_dims) >= 2
		&& sizeof...(rhs_dims) >= 2
		&& sizeof...(lhs_dims) == sizeof...(rhs_dims)
		&& [](
			std::array<usize, sizeof...(lhs_dims)> lhs_a,
			std::array<usize, sizeof...(lhs_dims)> rhs_a
		) -> bool {
			return std::equal(
				lhs_a.begin(), lhs_a.end() - 2,
				rhs_a.begin(), rhs_a.end() - 2
			);
		}(std::array{lhs_dims...},std::array{rhs_dims...})
		&& [](
			std::array<usize, sizeof...(lhs_dims)> lhs_a,
			std::array<usize, sizeof...(lhs_dims)> rhs_a
		) -> bool {
			return lhs_a[sizeof...(lhs_dims) - 1] == rhs_a[sizeof...(rhs_dims) - 2];
		}(std::array{lhs_dims...},std::array{rhs_dims...})
	)
	auto operator*(
		Matrix<T, lhs_dims...> const& lhs,
		Matrix<T, rhs_dims...> const& rhs
	) {
		using Product_Matrix = decltype([]<usize... dims>(std::index_sequence<dims...>) {
			auto constexpr lds = std::array{lhs_dims...};
			auto constexpr rds = std::array{rhs_dims...};
			auto constexpr n = sizeof...(lhs_dims);
			return Matrix<T, (
				dims < n - 2 ? lds[dims] : 
				dims == n - 2 ? lds[n-2] : 
				rds[n-1]
			)...>{};
		}(std::make_index_sequence<sizeof...(lhs_dims)>{}));
		auto product = Product_Matrix{};

		Matrix_Multiply_Impl<T>::multiply(lhs, rhs, product);
		return product;
	}
}
