#pragma once
#include <metatron/core/math/vector.hpp>

namespace metatron::math {
	auto inline constexpr morton_encode(u32 x, u32 y) -> u32 {
		auto constexpr spread_bits = [](u32 n) -> u32 {
			n = (n | (n << 16)) & 0x0000ffff;
			n = (n | (n << 8)) & 0x00ff00ff;
			n = (n | (n << 4)) & 0x0f0f0f0f;
			n = (n | (n << 2)) & 0x33333333;
			n = (n | (n << 1)) & 0x55555555;
			return n;
		};
		return (spread_bits(y) << 1) | spread_bits(x);
	};

	auto inline constexpr morton_decode(u32 x) -> math::Vector<u32, 2> {
		auto constexpr compact_bits = [](u32 n) -> u32 {
			n &= 0x55555555;
			n = (n | (n >> 1)) & 0x33333333;
			n = (n | (n >> 2)) & 0x0F0F0F0F;
			n = (n | (n >> 4)) & 0x00FF00FF;
			n = (n | (n >> 8)) & 0x0000FFFF;
			return n;
		};
		return {compact_bits(x), compact_bits(x >> 1)};
	}
}
