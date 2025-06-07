#pragma once
#include <metatron/core/math/vector.hpp>

namespace metatron::math {
	template<usize n>
	auto inline constexpr morton_encode(math::Vector<u32, n> x) -> u32 {
		if constexpr (n == 2) {
			auto constexpr spread_bits = [](u32 x) -> u32 {
				x = (x | (x << 16)) & 0x0000ffff;
				x = (x | (x << 8)) & 0x00ff00ff;
				x = (x | (x << 4)) & 0x0f0f0f0f;
				x = (x | (x << 2)) & 0x33333333;
				x = (x | (x << 1)) & 0x55555555;
				return x;
			};
			return (spread_bits(x[1]) << 1) | spread_bits(x[0]);
		} else if constexpr (n == 3) {
			auto constexpr spread_bits = [](u32 x) -> u32 {
				x = (x | (x << 16)) & 0x030000ff;
				x = (x | (x << 8)) & 0x0300f00f;
				x = (x | (x << 4)) & 0x030c30c3;
				x = (x | (x << 2)) & 0x09249249;
				return x;
			};
			return (spread_bits(x[2]) << 2) | (spread_bits(x[1]) << 1) | spread_bits(x[0]);
		}
	}

	template<usize n>
	auto inline constexpr morton_decode(u32 x) -> math::Vector<u32, n> {
		if constexpr (n == 2) {
			auto constexpr compact_bits = [](u32 x) -> u32 {
				x &= 0x55555555;
				x = (x | (x >> 1)) & 0x33333333;
				x = (x | (x >> 2)) & 0x0f0f0f0f;
				x = (x | (x >> 4)) & 0x00ff00ff;
				x = (x | (x >> 8)) & 0x0000ffff;
				return x;
			};
			return {compact_bits(x), compact_bits(x >> 1)};
		} else if constexpr (n == 3) {
			auto constexpr compact_bits = [](u32 x) -> u32 {
				x &= 0x09249249;
				x = (x | (x >> 2)) & 0x030c30c3;
				x = (x | (x >> 4)) & 0x0300f00f;
				x = (x | (x >> 8)) & 0x030000ff;
				x = (x | (x >> 16)) & 0x000000ff;
				return x;
			};
			return {compact_bits(x), compact_bits(x >> 1), compact_bits(x >> 2)};
		}
	}
}
