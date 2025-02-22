#pragma once
#include <metatron/core/math/matrix/types.hpp>

namespace metatron::math {
	namespace {
		template<typename T>
		struct Matrix_Multiply_Impl final { template<
				usize... lhs_dims,
				usize... rhs_dims,
				usize... product_dims,
				usize l_n = sizeof...(lhs_dims),
				usize r_n = sizeof...(rhs_dims),
				usize p_n = sizeof...(product_dims),
				usize h_n = std::max(usize(0), l_n == r_n ? p_n - 2 : p_n - 1),
				std::array<usize, l_n> lds = {lhs_dims...},
				std::array<usize, r_n> rds = {rhs_dims...},
				std::array<usize, p_n> pds = {product_dims...}
			>
			static void multiply(
				Matrix<T, lhs_dims...> const& lhs,
				Matrix<T, rhs_dims...> const& rhs,
				Matrix<T, product_dims...>& product
			) {
				if constexpr (h_n > 0) {
					for (auto i = 0; i < pds[0]; i++) {
						multiply(lhs[i], rhs[i], product[i]);
					}
				} else {
					if constexpr (l_n == 1) {
						for (auto i = 0; i < pds[0]; i++) {
							for (auto j = 0; j < lds[0]; j++) {
								product[i] += lhs[j] * rhs[j][i];
							}
						}
					} else if constexpr (r_n == 1) {
						for (auto i = 0; i < pds[0]; i++) {
							for (auto j = 0; j < rds[0]; j++) {
								product[i] += lhs[i][j] * rhs[j];
							}
						}
					} else {
						for (auto i = 0; i < pds[0]; i++) {
							for (auto k = 0; k < pds[1]; k++) {
								for (auto j = 0; j < lds[1]; j++) {
									product[i][k] += lhs[i][j] * rhs[j][k];
								}
							}
						}
					}
				}
			}
		};
	}

	template<
		typename T,
		usize... lhs_dims,
		usize... rhs_dims,
		usize l_n = sizeof...(lhs_dims),
		usize r_n = sizeof...(rhs_dims),
		usize shorter_n = std::min(l_n, r_n),
		usize longer_n = std::max(l_n, r_n),
		usize higher_n = std::max(usize(0), longer_n - (longer_n == 1 ? 1 : 2)),
		std::array<usize, l_n> lds = {lhs_dims...},
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
		Matrix<T, lhs_dims...> const& lhs,
		Matrix<T, rhs_dims...> const& rhs
	) {
		using Product_Matrix = decltype([]<usize... dims>(std::index_sequence<dims...>) {
			return Matrix<T, (
				dims < higher_n ? lds[dims] : 
				dims == higher_n ? (l_n < r_n ? rds[higher_n + 1] : lds[higher_n]) : 
				rds[higher_n + 1]
			)...>{};
		}(std::make_index_sequence<shorter_n>{}));
		auto product = Product_Matrix{};
		Matrix_Multiply_Impl<T>::multiply(lhs, rhs, product);
		return product;
	}
}
