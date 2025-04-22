#pragma once
#include <cstddef>
#include <cstdint>

namespace metatron::inline prelude {
	using i8 = std::int8_t;
	using i16 = std::int16_t;
	using i32 = std::int32_t;
	using i64 = std::int64_t;

	using u8 = std::uint8_t;
	using u16 = std::uint16_t;
	using u32 = std::uint32_t;
	using u64 = std::uint64_t;

	using byte = u8;
	using uptr = u64;
	using usize = std::size_t;

	using f32 = float;
	using f64 = double;
}
