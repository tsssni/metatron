#include <metatron/render/monte-carlo/volume-path.hpp>
#include <metatron/core/math/distribution/discrete.hpp>
#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/optional.hpp>
#include <metatron/geometry/shape/plane.hpp>

namespace metatron::mc {
	auto Volume_Path_Integrator::sample(
		Context ctx,
		accel::Acceleration const& accel,
		emitter::Emitter const& emitter,
		math::Sampler const& sampler
	) const -> std::optional<spectra::Stochastic_Spectrum> {
		auto lambda_u = sampler.generate_1d();
		auto Le = spectra::Stochastic_Spectrum{spectra::stochastic_samples, lambda_u};
		auto beta = spectra::Stochastic_Spectrum{spectra::stochastic_samples, lambda_u, 1.f};
		auto mis_s = beta;
		auto mis_e = beta;

		auto constexpr max_depth = 11uz;
		auto depth = 0uz;
		auto crossed = true;
		auto terminated = false;
		auto transmitted = false;

		auto scattered = false;
		auto scatter_pdf = 0.f;
		auto scatter_f = Le;

		auto trace_ctx = eval::Context{};
		auto history_trace_ctx = eval::Context{};
		trace_ctx.r = ctx.ray_differential.r;
		trace_ctx.L = Le;

		auto div_opt = std::optional<accel::Divider const*>{};
		auto intr_opt = std::optional<shape::Interaction>{};
		auto bsdf = std::unique_ptr<material::Bsdf>{};
		auto phase = (phase::Phase_Function const*)nullptr;

		auto& rt = *ctx.world_to_render;
		auto& ct = *ctx.render_to_camera;
		auto& mt = *ctx.medium_to_world;
		
		while (true) {
			depth += usize(scattered);
			if (terminated || depth >= max_depth) {
				break;
			}

			auto q = spectra::max(beta * math::guarded_div(1.f, spectra::avg(mis_s)));
			if (q < 1.f && depth > 1uz) {
				auto rr_u = sampler.generate_1d();
				if (rr_u > q) {
					terminated = true;
					continue;
				} else {
					beta /= q;
				}
			}

			if (scattered) {
				do {
					OPTIONAL_OR_BREAK(e_intr, emitter.sample(trace_ctx, sampler.generate_2d()));
					auto direct_ctx = trace_ctx;
					auto& lt = *e_intr.divider->local_to_world;

					direct_ctx = lt ^ (rt ^ direct_ctx);
					OPTIONAL_OR_BREAK(l_intr, e_intr.divider->light->sample(direct_ctx, sampler.generate_2d()));
					direct_ctx = rt | (lt | direct_ctx);

					l_intr.wi = rt | (lt | math::expand(l_intr.wi, 0.f));
					auto pe = e_intr.pdf * l_intr.pdf;
					if (std::abs(pe) < math::epsilon<f32>) {
						break;
					}

					auto ps = 0.f;
					auto f = spectra::Stochastic_Spectrum{};

					if (bsdf) {
						auto t = math::Transform{};
						t.config.rotation = math::Quaternion<f32>::from_rotation_between(direct_ctx.n, {0.f, 1.f, 0.f});
						auto wo = t | math::expand(history_trace_ctx.r.d, 0.f);
						auto wi = t | math::expand(l_intr.wi, 0.f);
						OPTIONAL_OR_BREAK(b_intr, (*bsdf)(wo, wi, Le));
						f = b_intr.f;
						ps = b_intr.pdf;
					} else if (phase) {
						OPTIONAL_OR_BREAK(p_intr, (*phase)(history_trace_ctx.r.d, l_intr.wi, Le));
						f = p_intr.f;
						ps = p_intr.pdf;
					} else {
						break;
					}

					auto medium = ctx.medium;
					auto medium_to_world = ctx.medium_to_world;
					auto& mt = *medium_to_world;
					auto terminated = false;

					auto mis_sd = mis_s * ps / pe;
					auto mis_ed = mis_s;
					auto betad = beta * (f / pe) / (scatter_f / scatter_pdf);

					while (true) {
						if (terminated || std::abs(l_intr.t) < math::epsilon<f32>) {
							break;
						}

						if (spectra::max(betad * math::guarded_div(1.f, spectra::avg(mis_sd + mis_ed))) < 0.05f) {
							auto q = 0.75f;
							if (sampler.generate_1d() > q) {
								spectra::clear(betad);
								terminated = true;
								continue;
							} else {
								betad /= q;
							}
						}

						div_opt = accel(direct_ctx.r);
						intr_opt = std::optional<shape::Interaction>{};

						if (!div_opt.has_value()) {
							terminated = true;
							continue;
						}
						auto& div = div_opt.value();
						auto& lt = *div->local_to_world;
						auto lr = lt ^ (rt ^ direct_ctx.r);
						intr_opt = (*div->shape)(lr);

						if (!intr_opt.has_value()) {
							terminated = true;
							continue;
						} else if (div->material) {
							spectra::clear(betad);
							terminated = true;
							continue;
						}
						auto& intr = intr_opt.value();
						intr.p = rt | (lt | math::expand(intr.p, 1.f));
						intr.n = math::normalize(rt | (lt | math::expand(intr.n, 0.f)));

						direct_ctx = mt ^ (rt ^ direct_ctx);
						OPTIONAL_OR_CALLBACK(m_intr, ctx.medium->sample(direct_ctx, intr.t, sampler.generate_1d()), {
							terminated = true;
							spectra::clear(betad);
							break;
						});
						direct_ctx = rt | (mt | direct_ctx);
						m_intr.p = rt | (mt | math::expand(direct_ctx.r.o, 1.f));
						l_intr.t -= m_intr.t;

						auto hit = m_intr.t >= intr.t;

						mis_sd *= m_intr.spectra_pdf / m_intr.pdf;
						mis_ed *= m_intr.spectra_pdf / m_intr.pdf;

						if (!hit) {
							betad *= m_intr.sigma_n * m_intr.transmittance / m_intr.pdf;
							intr.t -= m_intr.t;
							direct_ctx.r.o = m_intr.p;
							mis_sd *= (m_intr.sigma_n / m_intr.sigma_maj);
							continue;
						} else {
							betad *= m_intr.transmittance / m_intr.pdf;
							if (math::dot(-direct_ctx.r.d, intr.n) >= 0.f) {
								medium = div->interior_medium;
								medium_to_world = div->interior_transform;
							} else {
								medium = div->exterior_medium;
								medium_to_world = div->exterior_transform;
							}

							direct_ctx.r.o = intr.p + direct_ctx.r.d * 0.001f;
							break;
						}
					}

					auto mis_w = math::guarded_div(1.f, spectra::avg(mis_sd + mis_ed));
					Le += mis_w * betad * l_intr.Le;
				} while (false);
			}

			if (scattered || crossed) {
				div_opt = accel(trace_ctx.r);
				intr_opt = {};
				if (div_opt) {
					auto div = div_opt.value();
					auto& lt = *div->local_to_world;
					auto r = lt ^ (rt ^ trace_ctx.r);
					intr_opt = (*div->shape)(r, div->primitive);
				}
			}

			if (!intr_opt) {
				terminated = true;

				OPTIONAL_OR_CONTINUE(e_intr, emitter.sample_infinite(trace_ctx, sampler.generate_1d()));
				auto& lt = *e_intr.divider->local_to_world;

				trace_ctx = lt ^ (rt ^ trace_ctx);
				OPTIONAL_OR_CONTINUE(l_intr, (*e_intr.divider->light)(trace_ctx));
				trace_ctx = rt | (lt | trace_ctx);

				auto pl = e_intr.pdf * l_intr.pdf;
				mis_e *= pl;
				auto mis_w = (depth == 0uz && !transmitted) ? 1.f : math::guarded_div(1.f, spectra::avg(mis_s + mis_e));
				Le += beta * mis_w * l_intr.Le;
				continue;
			}

			auto& div = div_opt.value();
			auto& intr = intr_opt.value();
			auto& lt = *div->local_to_world;
			if (scattered || crossed) {
				intr.p = rt | (lt | math::expand(intr.p, 1.f));
				intr.n = math::normalize(rt | (lt | math::expand(intr.n, 0.f)));
			}

			trace_ctx = mt ^ (rt ^ trace_ctx);
			OPTIONAL_OR_CALLBACK(m_intr, ctx.medium->sample(trace_ctx, intr.t, sampler.generate_1d()), {
				terminated = true;
				continue;
			});
			trace_ctx = rt | (mt | trace_ctx);
			m_intr.p = rt | (mt | math::expand(m_intr.p, 1.f));

			auto hit = m_intr.t >= intr.t;

			beta *= m_intr.transmittance / m_intr.pdf;
			mis_s *= m_intr.spectra_pdf / m_intr.pdf;
			mis_e *= m_intr.spectra_pdf / m_intr.pdf;

			if (!hit) {
				auto mis_a = math::guarded_div(1.f, spectra::avg(mis_s));
				Le += mis_a * beta * m_intr.sigma_a * m_intr.Le;

				auto p_a = m_intr.sigma_a.value.front() / m_intr.sigma_maj.value.front();
				auto p_s = m_intr.sigma_s.value.front() / m_intr.sigma_maj.value.front();
				auto p_n = m_intr.sigma_n.value.front() / m_intr.sigma_maj.value.front();

				auto u = sampler.generate_1d();
				auto mode = math::Discrete_Distribution{std::array<f32, 3>{p_a, p_s, p_n}}.sample(u);
				if (mode == 0uz) {
					beta = m_intr.sigma_a / p_a;
					terminated = true;
				} else if (mode == 1uz) {
					phase = m_intr.phase;
					bsdf.reset();
					OPTIONAL_OR_CALLBACK(p_intr, phase->sample(trace_ctx, sampler.generate_2d()), {
						terminated = true;
						continue;
					});

					beta *= m_intr.sigma_s / p_s * p_intr.f / p_intr.pdf;
					mis_s *= (m_intr.sigma_s / m_intr.sigma_maj * p_intr.pdf) / (p_s * p_intr.pdf);
					mis_e = mis_s / p_intr.pdf;

					scattered = true;
					crossed = false;
					scatter_f = p_intr.f;
					scatter_pdf = p_intr.pdf;
					history_trace_ctx = trace_ctx;
					trace_ctx = {{m_intr.p, p_intr.wi}, {}, Le};
				} else {
					intr.t -= m_intr.t;
					trace_ctx.r.o = m_intr.p;
					beta = m_intr.sigma_n / p_n;
					mis_s *= (m_intr.sigma_n / m_intr.sigma_maj) / p_n;
					mis_e /= p_n;
					transmitted = true;
					scattered = false;
					crossed = false;
				}
				continue;
			}

			if (!ctx.ray_differential.differentiable) {
				auto rd = ct ^ ctx.default_differential;
				auto st = math::Transform{};
				st.config.rotation = math::Quaternion<f32>::from_rotation_between(rd.r.d, math::normalize(intr.p));
				ctx.ray_differential = st | rd;
			}
			
			auto local_differential = lt ^ (rt ^ ctx.ray_differential);
			auto tangent = shape::Plane{intr.p, intr.n};
			OPTIONAL_OR_BREAK(d_intr, tangent(local_differential.r));
			OPTIONAL_OR_BREAK(dx_intr, tangent(local_differential.rx));
			OPTIONAL_OR_BREAK(dy_intr, tangent(local_differential.ry));
			
			auto dpdx = dx_intr.p - d_intr.p;
			auto dpdy = dy_intr.p - d_intr.p;
			auto dpduv =  math::transpose(math::Matrix<f32, 2, 3>{intr.dpdu, intr.dpdv});
			auto duvdx = math::least_squares(dpduv, dpdx);
			auto duvdy = math::least_squares(dpduv, dpdy);

			auto tcoord = material::Coordinate{intr.uv, duvdx[0], duvdy[0], duvdx[1], duvdy[1]};
			OPTIONAL_OR_CALLBACK(mat_intr, div->material->sample(trace_ctx, tcoord), {
				terminated = true;
				continue;
			});
			ctx.ray_differential.differentiable = false;
			bsdf = std::move(mat_intr.bsdf);
			phase = nullptr;

			if (spectra::max(mat_intr.Le) > math::epsilon<f32>) {
				do {
					OPTIONAL_OR_BREAK(e_intr, emitter(trace_ctx, *div->Le));
					auto& div = *e_intr.divider;
					auto& lt = *div.local_to_world;

					trace_ctx = lt ^ (rt ^ trace_ctx);
					OPTIONAL_OR_BREAK(l_intr, (*e_intr.divider->light)(trace_ctx));
					trace_ctx = rt | (lt | trace_ctx);

					auto pe = e_intr.pdf * l_intr.pdf;
					mis_e *= pe;
					auto mis_w = math::guarded_div(1.f, spectra::avg(mis_s + mis_e));
					Le += mis_w * beta * mat_intr.Le;
				} while (false);
			}

			auto bt = math::Transform{};
			bt.config.rotation = math::Quaternion<f32>::from_rotation_between(intr.n, {0.f, 1.f, 0.f});
			auto uc = sampler.generate_1d();
			auto u = sampler.generate_2d();

			trace_ctx = bt | trace_ctx;
			OPTIONAL_OR_CALLBACK(b_intr, bsdf->sample(trace_ctx, {uc, u[0], u[1]}), {
				terminated = true;
				continue;
			});
			trace_ctx = bt ^ trace_ctx;
			b_intr.wi = bt ^ math::expand(b_intr.wi, 0.f);

			auto flip_n = 1.f;
			if (math::dot(-trace_ctx.r.d, b_intr.wi) < 0.f) {
				if (math::dot(b_intr.wi, intr.n) <= 0.f) {
					ctx.medium = div->interior_medium;
					ctx.medium_to_world = div->interior_transform;
					flip_n = -1.f;
				} else {
					ctx.medium = div->exterior_medium;
					ctx.medium_to_world = div->exterior_transform;
				}
			}
			auto trace_n = flip_n * intr.n;
			auto trace_p = intr.p + 0.001f * b_intr.wi;

			scattered = trace_ctx.r.d != b_intr.wi;
			crossed = !scattered;
			history_trace_ctx = trace_ctx;
			trace_ctx = {{trace_p, b_intr.wi}, trace_n, Le};
			if (scattered) {
				beta *= b_intr.f / b_intr.pdf;
				mis_e = mis_s / b_intr.pdf;
				scatter_f = b_intr.f;
				scatter_pdf = b_intr.pdf;
			} else {
				transmitted = true;
				spectra::clear(mis_e);
			}
		}

		return Le;
	}
}
