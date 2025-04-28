#pragma once
#include <metatron/core/math/matrix.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <print>

namespace metatron::math {
	// forward declaration to support the declaration of 0d piecewise distribution
	template<usize... dims>
	struct Piecewise_Distribution;

	template<usize first_dim, usize... rest_dims>
	requires (first_dim > 0 && (... && (rest_dims > 0)))
	struct Piecewise_Distribution<first_dim, rest_dims...> final {
		using Element = std::conditional_t<sizeof...(rest_dims) == 0, f32, Piecewise_Distribution<rest_dims...>>;
		auto static constexpr dimensions = std::array<usize, 1 + sizeof...(rest_dims)>{first_dim, rest_dims...};

		Piecewise_Distribution() = default;

		Piecewise_Distribution(math::Matrix<f32, first_dim, rest_dims...>&& matrix) {
			*this = std::move(matrix);
		}

		auto operator=(math::Matrix<f32, first_dim, rest_dims...>&& matrix) -> Piecewise_Distribution& {
			auto constexpr n = dimensions.size();
			cdf[0] = 0.f;
			sum = 0.f;

			for (auto i = 0uz; i < first_dim; i++) {
				rows[i] = std::move(matrix[i]);
				if constexpr (n == 1uz) {
					cdf[i + 1] = rows[i];
				} else {
					cdf[i + 1] = rows[i].sum;
				}
				cdf[i + 1] = cdf[i + 1] / f32(first_dim) + cdf[i];
			}
			sum = cdf.back();

			for (auto i = 1uz; i <= first_dim; i++) {
				if (sum == 0.f) {
					cdf[i] = f32(i) / f32(first_dim);
				} else {
					cdf[i] /= sum;
				}
			}

			return *this;
		}

		auto sample(math::Vector<f32, dimensions.size()> const& u) -> math::Vector<f32, dimensions.size()> {
			auto constexpr n = dimensions.size();
			auto idx = 1uz;
			for (; idx < first_dim && cdf[idx] <= u[n - 1]; idx++) {}
			idx--;

			auto t = math::guarded_div(u[n - 1] - cdf[idx], cdf[idx + 1uz] - cdf[idx]);
			auto p = (f32(idx) + t) / f32(first_dim);

			if constexpr (n == 1uz) {
				return {p};
			} else {
				return {rows[idx].sample(math::shrink(u)), p};
			}
		}

		auto pdf(math::Vector<f32, dimensions.size()> const& u) -> f32 {
			auto constexpr n = dimensions.size();
			auto idx = usize(u[n - 1] * f32(first_dim));
			auto p = (cdf[idx + 1] - cdf[idx]) * f32(first_dim);
			if constexpr (n == 1uz) {
				return p;
			} else {
				return rows[idx].pdf(math::shrink(u)) * p;
			}
		}

	private:
		std::array<Element, first_dim> rows{};
		std::array<f32, first_dim + 1uz> cdf{};
		f32 sum{};

		template<usize... dims>
		friend struct Piecewise_Distribution;
	};
}
