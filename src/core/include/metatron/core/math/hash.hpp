#pragma once

namespace metatron::math {
	auto inline mix_bits(u64 x) -> u64 {
		x ^= (x >> 31);
		x *= 0x7fb5d329728ea185;
		x ^= (x >> 27);
		x *= 0x81dadef4bc2dd44d;
		x ^= (x >> 33);
		return x;
	}

	auto inline biject_permute(u32 i, u32 n, u32 seed) -> u32 {
		// set 1 for bits below highest 1
		auto w = n - 1;
		w |= w >> 1;
		w |= w >> 2;
		w |= w >> 4;
		w |= w >> 8;
		w |= w >> 16;

		do {
			i ^= seed;
			i *= 0xe170893d;
			i ^= seed >> 16;
			i ^= (i & w) >> 4;
			i ^= seed >> 8;
			i *= 0x0929eb3f;
			i ^= seed >> 23;
			i ^= (i & w) >> 1;
			i *= 1 | seed >> 27;
			i *= 0x6935fa69;
			i ^= (i & w) >> 11;
			i *= 0x74dcb303;
			i ^= (i & w) >> 2;
			i *= 0x9e501cc3;
			i ^= (i & w) >> 2;
			i *= 0xc860a3df;
			i &= w;
			i ^= i >> 5;
		} while (i >= n);

		return (i + seed) % n;
	}
}
