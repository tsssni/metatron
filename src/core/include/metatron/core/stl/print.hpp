#pragma once
#include <metatron/core/math/matrix.hpp>
#include <print>
#include <format>

template<typename T, mtt::usize... dims>
struct std::formatter<mtt::math::Matrix<T, dims...>> {
	using M = mtt::math::Matrix<T, dims...>;

	constexpr auto parse(std::format_parse_context& ctx) {
		return ctx.begin();
	}

	auto format(const mtt::math::Matrix<T, dims...>& matrix, std::format_context& ctx) const {
		auto constexpr size = sizeof...(dims);
		auto out = std::format_to(ctx.out(), size == 1 ? "[" : "[\n");
		constexpr auto first_dim = M::dimensions[0];
		for (auto i = 0uz; i < first_dim; i++) {
			if (i != 0 && size == 1) {
				out = std::format_to(out, ", {}", matrix[i]);
			} else {
				out = std::format_to(out, "{}", matrix[i]);
			}
		}
		return std::format_to(ctx.out(), "]\n");
	}
};

