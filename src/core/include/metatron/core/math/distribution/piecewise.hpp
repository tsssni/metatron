#pragma once
#include <metatron/core/math/matrix.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/arithmetic.hpp>

namespace metatron::math {
	// forward declaration to support the declaration of 0d piecewise distribution
	template<usize... dims>
	struct Piecewise_Distribution;

	template<usize first_dim, usize... rest_dims>
	requires (first_dim > 0 && (... && (rest_dims > 0)))
	struct Piecewise_Distribution<first_dim, rest_dims...> final {
		using Element = std::conditional_t<sizeof...(rest_dims) == 0, f32, Piecewise_Distribution<rest_dims...>>;
		auto static constexpr dimensions = std::array<usize, 1 + sizeof...(rest_dims)>{first_dim, rest_dims...};
		auto static constexpr n = dimensions.size();

		Piecewise_Distribution() = default;

		Piecewise_Distribution(
			Matrix<f32, first_dim, rest_dims...>&& matrix, 
			Vector<f32, n>&& low,
			Vector<f32, n>&& high
		) {
			cdf[0] = 0.f;
			integral = 0.f;
			this->low = low[n - 1];
			this->high = high[n - 1];

			for (auto i = 0uz; i < first_dim; i++) {
				if constexpr (n == 1uz) {
					rows[i] = math::abs(matrix[i]);
					cdf[i + 1] = rows[i];
				} else {
					rows[i] = Element{std::move(matrix[i]), shrink(low), shrink(high)};
					cdf[i + 1] = rows[i].integral;
				}
				cdf[i + 1] = cdf[i + 1] * (this->high - this->low) / f32(first_dim) + cdf[i];
			}

			integral = cdf.back();
			for (auto i = 1uz; i <= first_dim; i++) {
				if (integral == 0.f) {
					cdf[i] = f32(i) / f32(first_dim);
				} else {
					cdf[i] /= integral;
				}
			}
		}

		auto sample(Vector<f32, dimensions.size()> const& u) const -> Vector<f32, dimensions.size()> {
			auto idx = 1uz;
			for (; idx < first_dim && cdf[idx] <= u[n - 1]; idx++) {}
			idx--;

			auto t = guarded_div(u[n - 1] - cdf[idx], cdf[idx + 1uz] - cdf[idx]);
			auto p = std::lerp(low, high, (f32(idx) + t) / f32(first_dim));

			if constexpr (n == 1uz) {
				return {p};
			} else {
				return {rows[idx].sample(shrink(u)), p};
			}
		}

		auto pdf(Vector<f32, dimensions.size()> const& p) const -> f32 {
			auto idx = usize((p[n - 1] - low) / (high - low) * f32(first_dim));
			auto prob = (cdf[idx + 1] - cdf[idx]) * f32(first_dim) / (high - low);
			if constexpr (n == 1uz) {
				return prob;
			} else {
				return rows[idx].pdf(shrink(p)) * prob;
			}
		}

	private:
		std::array<Element, first_dim> rows{};
		std::array<f32, first_dim + 1uz> cdf{};
		f32 low{};
		f32 high{};
		f32 integral{};

		template<usize... dims>
		friend struct Piecewise_Distribution;
	};
}
