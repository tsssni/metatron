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
		auto terminated = false;

		auto scattered = false;
		auto scatter_pdf = 0.f;
		auto scatter_f = Le;
		auto scatter_ctx = eval::Context{};
		scatter_ctx.r = ctx.ray_differential.r;
		scatter_ctx.L = Le;

		auto div_opt = std::optional<accel::Divider const*>{};
		auto intr_opt = std::optional<shape::Interaction>{};
		auto bsdf = std::unique_ptr<material::Bsdf>{};
		auto phase = std::unique_ptr<phase::Phase_Function>{};

		auto& rt = *ctx.world_to_render;
		auto& ct = *ctx.render_to_camera;
		
		while (true) {
			depth += usize(scattered);
			if (terminated || depth >= max_depth) {
				break;
			}

			auto q = spectra::max(beta / spectra::avg(mis_s));
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
					OPTIONAL_OR_BREAK(e_intr, emitter.sample(scatter_ctx, sampler.generate_2d()));
					auto direct_ctx = scatter_ctx;
					auto& lt = *e_intr.divider->local_to_world;

					direct_ctx.p = lt ^ (rt ^ math::Vector<f32, 4>{direct_ctx.p, 1.f});
					direct_ctx.n = lt ^ (rt ^ math::Vector<f32, 4>{direct_ctx.n, 0.f});
					OPTIONAL_OR_BREAK(l_intr, e_intr.divider->light->sample(direct_ctx, sampler.generate_2d()));
					direct_ctx.p = rt | (lt | math::Vector<f32, 4>{direct_ctx.p, 1.f});
					direct_ctx.n = rt | (lt | math::Vector<f32, 4>{direct_ctx.n, 0.f});

					l_intr.wi = lt | l_intr.wi;
					auto pe = e_intr.pdf * l_intr.pdf;

					auto ps = 0.f;
					auto f = spectra::Stochastic_Spectrum{};

					if (bsdf) {
						auto t = math::Transform{};
						t.config.rotation = math::Quaternion<f32>::from_rotation_between(direct_ctx.n, {0.f, 1.f, 0.f});
						auto wo = t | ctx.ray_differential.r.d;
						auto wi = t | l_intr.wi;
						OPTIONAL_OR_BREAK(b_intr, (*bsdf)(wo, wi, Le));
						f = b_intr.f;
						ps = b_intr.pdf;
					} else if (phase) {
						OPTIONAL_OR_BREAK(p_intr, (*phase)(ctx.ray_differential.r.d, l_intr.wi, Le));
						f = p_intr.f;
						ps = p_intr.pdf;
					} else {
						break;
					}

					auto medium = ctx.medium;
					auto medium_to_world = ctx.medium_to_world;
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
						intr_opt = {};

						if (!div_opt.has_value()) {
							terminated = true;
							continue;
						}
						auto& div = div_opt.value();
						auto& lt = *div->local_to_world;
						intr_opt = (*div->shape)(lt ^ (rt ^ direct_ctx.r));

						if (!intr_opt.has_value()) {
							terminated = true;
							continue;
						} else if (div->material) {
							spectra::clear(betad);
							terminated = true;
							continue;
						}
						auto& intr = intr_opt.value();

						while (ctx.medium) {
							auto& mt = *medium_to_world;
							direct_ctx.r = mt ^ (rt ^ direct_ctx.r);
							OPTIONAL_OR_CALLBACK(m_intr, ctx.medium->sample(direct_ctx, intr.t, sampler.generate_1d()), {
								terminated = true;
								spectra::clear(betad);
								break;
							});
							direct_ctx.r = rt | (mt | direct_ctx.r);
							l_intr.t -= m_intr.t;

							auto hit = m_intr.t == intr.t;
							auto sigma_maj = m_intr.sigma_a + m_intr.sigma_s + m_intr.sigma_n;
							auto mis_t = hit ? m_intr.transmittance : sigma_maj * m_intr.transmittance;

							beta *= m_intr.transmittance / m_intr.pdf;
							mis_sd *= mis_t / m_intr.pdf;
							mis_ed *= mis_t / m_intr.pdf;

							if (!hit) {
								intr.t -= m_intr.t;
								direct_ctx.r.o = m_intr.p;
								mis_sd *= (m_intr.sigma_n / sigma_maj);
								continue;
							} else {
								intr.p = rt | (lt | math::Vector<f32, 4>{intr.p, 1.f});
								intr.n = rt | (lt | math::Vector<f32, 4>{intr.n, 0.f});

								if (math::dot(-direct_ctx.r.d, intr.n) >= 0.f) {
									medium = div->interior_medium;
									medium_to_world = div->interior_transform;
								} else {
									medium = div->exterior_medium;
									medium_to_world = div->exterior_transform;
								}

								direct_ctx.r.o = intr.p + direct_ctx.r.d * 0.001f;
								direct_ctx.p = direct_ctx.r.o;
								break;
							}
						}
					}

					auto mis_w = math::guarded_div(1.f, spectra::avg(mis_sd + mis_ed));
					Le += mis_w * betad * l_intr.Le;
				} while (false);
				
				ctx.ray_differential.r = scatter_ctx.r;
				ctx.ray_differential.r.o += ctx.ray_differential.r.d * 0.001f;
				ctx.ray_differential.differentiable = false;
			}

			if (scattered || depth == 0uz) {
				div_opt = accel(ctx.ray_differential.r);
				intr_opt = {};
				if (div_opt) {
					auto div = div_opt.value();
					auto& lt = *div->local_to_world;
					auto r = lt ^ (rt ^ ctx.ray_differential.r);
					intr_opt = (*div->shape)(r, div->primitive);
				}
			}

			if (!intr_opt) {
				terminated = true;

				OPTIONAL_OR_CONTINUE(e_intr, emitter.sample_infinite(scatter_ctx, sampler.generate_1d()));
				auto inf_ctx = scatter_ctx;
				auto& lt = *e_intr.divider->local_to_world;

				inf_ctx.r.d = lt ^ (rt ^ inf_ctx.r.d);
				inf_ctx.n = lt ^ (rt ^ inf_ctx.n);
				OPTIONAL_OR_CONTINUE(l_intr, (*e_intr.divider->light)(inf_ctx));
				inf_ctx.r.d = rt | (lt | inf_ctx.r.d);
				inf_ctx.n = rt | (lt | inf_ctx.n);

				auto pl = e_intr.pdf * l_intr.pdf;
				mis_e *= pl;
				auto mis_w = depth == 0uz ? 1.f : math::guarded_div(1.f, spectra::avg(mis_s + mis_e));
				Le = beta * mis_w * l_intr.Le;
				continue;
			}

			auto& div = div_opt.value();
			auto& intr = intr_opt.value();
			auto& lt = *div->local_to_world;

			if (ctx.medium) {
				auto& mt = *ctx.medium_to_world;
				auto m_ctx = eval::Context{ctx.ray_differential.r.o, {}, ctx.ray_differential.r, Le};
				m_ctx.r = mt ^ (rt ^ m_ctx.r);
				OPTIONAL_OR_CALLBACK(m_intr, ctx.medium->sample(m_ctx, intr.t, sampler.generate_1d()), {
					terminated = true;
					continue;
				});
				m_ctx.r = rt | (mt | m_ctx.r);
				m_intr.p = rt | (mt | math::Vector<f32, 4>{m_intr.p, 1.f});

				auto hit = m_intr.t == intr.t;
				auto sigma_maj = m_intr.sigma_a + m_intr.sigma_s + m_intr.sigma_n;
				auto mis_t = hit ? m_intr.transmittance : sigma_maj * m_intr.transmittance;

				beta *= m_intr.transmittance / m_intr.pdf;
				mis_s *= mis_t / m_intr.pdf;
				mis_e *= mis_t / m_intr.pdf;

				if (!hit) {
					auto mis_a = math::guarded_div(1.f, spectra::avg(mis_s));
					Le += mis_a * beta * m_intr.sigma_a * m_intr.Le;

					auto p_a = m_intr.sigma_a.value[0] / sigma_maj.value[0];
					auto p_s = m_intr.sigma_s.value[0] / sigma_maj.value[0];
					auto p_n = m_intr.sigma_n.value[0] / sigma_maj.value[0];

					auto u = sampler.generate_1d();
					auto mode = math::Discrete_Distribution{std::array<f32, 3>{p_a, p_s, p_n}}.sample(u);
					if (mode == 0uz) {
						beta /= p_a;
						terminated = true;
					} else if (mode == 1uz) {
						phase = std::move(m_intr.phase);
						bsdf.reset();
						OPTIONAL_OR_CALLBACK(p_intr, phase->sample(m_ctx, sampler.generate_2d()), {
							terminated = true;
							continue;
						});

						beta *= m_intr.sigma_s / p_s * p_intr.f / p_intr.pdf;
						mis_s *= (m_intr.sigma_s / sigma_maj * p_intr.pdf) / (p_s * p_intr.pdf);
						mis_e = mis_s / p_intr.pdf;

						scattered = true;
						scatter_f = p_intr.f;
						scatter_pdf = p_intr.pdf;
						scatter_ctx = {m_intr.p, {}, {m_intr.p, p_intr.wi}, Le};
					} else {
						intr.t -= m_intr.t;
						ctx.ray_differential.r.o = m_intr.p;
						beta /= p_n;
						mis_s *= (m_intr.sigma_n / sigma_maj) / p_n;
						mis_e /= p_n;
						scattered = false;
					}
					continue;
				}
			}

			if (!div->material) {
				if (math::dot(-ctx.ray_differential.r.d, intr.n) >= 0.f) {
					ctx.medium = div->interior_medium;
					ctx.medium_to_world = div->interior_transform;
				} else {
					ctx.medium = div->exterior_medium;
					ctx.medium_to_world = div->exterior_transform;
				}
				ctx.ray_differential.r.o = intr.p + ctx.ray_differential.r.d * 0.001f;
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
			OPTIONAL_OR_CALLBACK(mat_intr, div->material->sample(scatter_ctx, tcoord), {
				terminated = true;
				continue;
			});
			bsdf = std::move(mat_intr.bsdf);
			phase.reset();

			if (spectra::max(mat_intr.Le) > math::epsilon<f32>) {
				do {
					OPTIONAL_OR_BREAK(e_intr, emitter(scatter_ctx, *div->Le));
					auto emit_ctx = scatter_ctx;
					auto& div = *e_intr.divider;
					auto& lt = *div.local_to_world;

					emit_ctx.r.d = lt ^ (rt ^ emit_ctx.r.d);
					emit_ctx.n = lt ^ (rt ^ emit_ctx.n);
					OPTIONAL_OR_BREAK(l_intr, (*e_intr.divider->light)(emit_ctx));
					emit_ctx.r.d = rt | (lt | emit_ctx.r.d);
					emit_ctx.n = rt | (lt | emit_ctx.n);

					auto pe = e_intr.pdf * l_intr.pdf;
					mis_e *= pe;
					auto mis_w = math::guarded_div(1.f, spectra::avg(mis_s + mis_e));
					Le += mis_w * beta * mat_intr.Le;
				} while (false);
			}

			intr.p = rt | (lt | math::Vector<f32, 4>{intr.p, 1.f});
			intr.n = rt | (lt | math::Vector<f32, 4>{intr.n, 0.f});

			auto bt = math::Transform{};
			bt.config.rotation = math::Quaternion<f32>::from_rotation_between(intr.n, {0.f, 1.f, 0.f});
			auto uc = sampler.generate_1d();
			auto u = sampler.generate_2d();

			scatter_ctx.r = bt | scatter_ctx.r;
			OPTIONAL_OR_CALLBACK(b_intr, bsdf->sample(scatter_ctx, {uc, u[0], u[1]}), {
				terminated = true;
				continue;
			});
			scatter_ctx.r = bt ^ scatter_ctx.r;
			b_intr.wi = bt ^ math::Vector<f32, 4>{b_intr.wi, 0.f};

			auto flip_n = 1.f;
			if (math::dot(-scatter_ctx.r.d, b_intr.wi) < 0.f) {
				if (math::dot(b_intr.wi, intr.n) <= 0.f) {
					ctx.medium = div->interior_medium;
					ctx.medium_to_world = div->interior_transform;
					flip_n = -1.f;
				} else {
					ctx.medium = div->exterior_medium;
					ctx.medium_to_world = div->exterior_transform;
				}
			}

			beta *= b_intr.f / b_intr.pdf;
			mis_e = mis_s / b_intr.pdf;
			scattered = true;
			scatter_f = b_intr.f;
			scatter_pdf = b_intr.pdf;
			scatter_ctx = {intr.p, flip_n * intr.n, {intr.p, b_intr.wi}, Le};
		}

		return Le;
	}
}
