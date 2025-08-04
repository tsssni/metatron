#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/math/transform.hpp>
#include <metatron/resource/spectra/stochastic.hpp>

namespace mtt::bsdf {
	struct Bsdf;
}

namespace mtt::phase {
	struct Phase_Function;
}

namespace mtt::eval {
	struct Context final {
		math::Ray r{};
		math::Vector<f32, 3> n{};
		spectra::Stochastic_Spectrum spec{};
		bool inside;
	};

	auto inline operator|(math::Transform const& t, Context const& ctx) -> Context {
		auto result = ctx;
		result.r = t | result.r;
		result.n = t | result.n;
		return result;
	}

	auto inline operator^(math::Transform const& t, Context const& ctx) -> Context {
		auto result = ctx;
		result.r = t ^ result.r;
		result.n = t ^ result.n;
		return result;
	}

	auto inline operator|(math::Transform::Chain&& chain, Context const& ctx) -> Context {
		auto result = ctx;
		result.r = chain | result.r;
		result.n = chain | result.n;
		return result;
	}

	auto inline operator^(math::Transform::Chain&& chain, Context const& ctx) -> Context {
		auto result = ctx;
		result.r = chain ^ result.r;
		result.n = chain ^ result.n;
		return result;
	}
}
