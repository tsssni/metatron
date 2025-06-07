#include <metatron/render/monte-carlo/volume-path.hpp>
#include <metatron/resource/shape/plane.hpp>
#include <metatron/core/math/distribution/discrete.hpp>
#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/optional.hpp>

namespace metatron::monte_carlo {
	auto Volume_Path_Integrator::sample(
		Context int_ctx,
		accel::Acceleration const& accel,
		emitter::Emitter const& emitter,
		math::Sampler const& sampler
	) const -> std::optional<spectra::Stochastic_Spectrum> {
		auto lambda_u = sampler.generate_1d();
		auto L_e = spectra::Stochastic_Spectrum{spectra::stochastic_samples, lambda_u};
		auto beta = L_e; beta = 1.f;
		auto mis_s = L_e; mis_s = 1.f;
		auto mis_e = L_e; mis_e = 0.f;

		auto depth = 0uz;
		auto max_depth = int_ctx.max_depth;
		auto crossed = true;
		auto terminated = false;
		auto inside = false;

		auto scattered = false;
		auto scatter_pdf = 0.f;
		auto scatter_f = L_e;

		auto bsdf = std::unique_ptr<bsdf::Bsdf>{};
		auto phase = std::unique_ptr<phase::Phase_Function>{};
		auto trace_ctx = eval::Context{};
		auto history_trace_ctx = eval::Context{};
		trace_ctx.r = int_ctx.ray_differential.r;
		trace_ctx.L = L_e;

		auto acc_opt = std::optional<accel::Interaction>{};
		auto* medium = int_ctx.medium;
		auto* medium_to_world = int_ctx.medium_to_world;
		auto& rdiff = int_ctx.ray_differential;
		auto& ddiff = int_ctx.default_differential;
		auto& rt = *int_ctx.world_to_render;
		auto& ct = *int_ctx.render_to_camera;
		
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

			[&]() {
				if (!scattered) {
					return;
				}

				auto direct_ctx = trace_ctx;
				METATRON_OPT_OR_RETURN(e_intr, emitter.sample(direct_ctx, sampler.generate_1d()));
				auto& lt = *e_intr.divider->local_to_world;

				auto l_ctx = lt ^ rt ^ direct_ctx;
				METATRON_OPT_OR_RETURN(l_intr, e_intr.divider->light->sample(l_ctx, sampler.generate_2d()));

				l_intr.p = rt | lt | math::expand(l_intr.p, 1.f);
				l_intr.wi = math::normalize(rt | lt | math::expand(l_intr.wi, 0.f));
				direct_ctx.r.d = l_intr.wi;
				auto p_e = e_intr.pdf * l_intr.pdf;
				if (std::abs(p_e) < math::epsilon<f32>) {
					return;
				}

				auto p_s = 0.f;
				auto f = spectra::Stochastic_Spectrum{};

				if (trace_ctx.bsdf) {
					auto t = math::Transform{{}, {1.f},
						math::Quaternion<f32>::from_rotation_between(direct_ctx.n, {0.f, 1.f, 0.f})
					};
					auto wo = t | math::expand(history_trace_ctx.r.d, 0.f);
					auto wi = t | math::expand(l_intr.wi, 0.f);
					METATRON_OPT_OR_RETURN(b_intr, (*trace_ctx.bsdf)(wo, wi, L_e));
					f = b_intr.f * std::abs(math::dot(l_intr.wi, direct_ctx.n));
					p_s = b_intr.pdf;
				} else if (trace_ctx.phase) {
					METATRON_OPT_OR_RETURN(p_intr, (*trace_ctx.phase)(history_trace_ctx.r.d, l_intr.wi, L_e));
					f = p_intr.f;
					p_s = p_intr.pdf;
				} else {
					return;
				}

				auto* dedium = medium;
				auto* direct_to_world = medium_to_world;

				auto acc_opt = std::optional<accel::Interaction>{};
				auto terminated = false;
				auto crossed = true;
				auto inmed = inside;

				auto gamma = beta * (f / p_e) / (scatter_f / scatter_pdf);
				auto mis_d = mis_s * p_s / p_e * float(!light::Light::is_delta(*e_intr.divider->light));
				auto mis_l = mis_s;

				while (true) {
					if (terminated || l_intr.t < 0.001f) {
						break;
					}

					if (spectra::max(gamma * math::guarded_div(1.f, spectra::avg(mis_d + mis_l))) < 0.05f) {
						auto q = 0.25f;
						if (sampler.generate_1d() > q) {
							gamma = 0.f;
							terminated = true;
							continue;
						} else {
							gamma /= q;
						}
					}

					if (crossed) {
						acc_opt = accel(direct_ctx.r, direct_ctx.n);
						if (!acc_opt) {
							terminated = true;
							continue;
						}
						auto& acc = acc_opt.value();

						if (!acc.intr_opt) {
							terminated = true;
							continue;
						}
						auto& intr = acc_opt->intr_opt.value();

						auto& div = *acc.divider;
						intr.p = rt | lt | math::expand(intr.p, 1.f);
						intr.n = math::normalize(rt | lt | intr.n);
						inmed = math::dot(-direct_ctx.r.d, intr.n) < 0.f;
						intr.n *= inmed ? -1.f : 1.f;

						auto close_to_light = math::length(intr.p - l_intr.p) < 0.001f;
						if (!close_to_light && !material::Material::is_interface(*div.material)) {
							terminated = true;
							gamma = 0.f;
							continue;
						} else if (close_to_light) {
							auto rd = ct ^ ddiff;
							auto st = math::Transform{{}, {1.f},
								math::Quaternion<f32>::from_rotation_between(rd.r.d, math::normalize(intr.p))
							};
							rd = st | rd;

							auto ldiff = lt ^ rt ^ rd;
							auto tangent = shape::Plane{intr.p, intr.n};
							#define METATRON_DSTOP {terminated = true; gamma = 0.f; std::printf("\n===\n"); continue;}
							METATRON_OPT_OR_CALLBACK(d_intr, tangent(ldiff.r), METATRON_DSTOP);
							METATRON_OPT_OR_CALLBACK(dx_intr, tangent(ldiff.rx), METATRON_DSTOP);
							METATRON_OPT_OR_CALLBACK(dy_intr, tangent(ldiff.ry), METATRON_DSTOP);
							
							auto dpdx = dx_intr.p - d_intr.p;
							auto dpdy = dy_intr.p - d_intr.p;
							auto dpduv =  math::transpose(math::Matrix<f32, 2, 3>{intr.dpdu, intr.dpdv});
							auto duvdx = math::least_squares(dpduv, dpdx);
							auto duvdy = math::least_squares(dpduv, dpdy);

							auto tcoord = texture::Coordinate{intr.uv, duvdx[0], duvdy[0], duvdx[1], duvdy[1]};
							METATRON_OPT_OR_CALLBACK(mat_intr, div.material->sample(direct_ctx, tcoord), METATRON_DSTOP);
							l_intr.L = mat_intr.L;
						}
					}

					auto& acc = acc_opt.value();
					auto& div = acc.divider;
					auto& intr = acc.intr_opt.value();

					auto& mt = *direct_to_world;
					auto m_ctx = mt ^ rt ^ direct_ctx;
					METATRON_OPT_OR_CALLBACK(m_intr, dedium->sample(m_ctx, intr.t, sampler.generate_1d()), {
						gamma = 0.f;
						terminated = true;
						break;
					});
					m_intr.p = rt | mt | math::expand(m_intr.p, 1.f);
					l_intr.t -= m_intr.t;

					auto hit = m_intr.t >= intr.t;

					gamma *= m_intr.transmittance / m_intr.pdf;
					mis_d *= m_intr.spectra_pdf / m_intr.pdf;
					mis_l *= m_intr.spectra_pdf / m_intr.pdf;

					if (!hit) {
						gamma *= m_intr.sigma_n;
						mis_d *= m_intr.sigma_n / m_intr.sigma_maj;
						intr.t -= m_intr.t;
						direct_ctx.r.o = m_intr.p;
						crossed = false;
					} else {
						dedium = !inmed ? div->interior_medium : div->exterior_medium;
						direct_to_world = !inmed ? div->interior_transform : div->exterior_transform;
						direct_ctx.r.o = intr.p - 0.001f * intr.n;
						crossed = true;
					}
					continue;
				}

				auto mis_u = math::guarded_div(1.f, spectra::avg(mis_d + mis_l));
				L_e += gamma * mis_u * l_intr.L;
			}();

			if (scattered || crossed) {
				acc_opt = accel(trace_ctx.r, trace_ctx.n);
			}

			if (!acc_opt || !acc_opt.value().intr_opt) {
				terminated = true;

				METATRON_OPT_OR_CONTINUE(e_intr, emitter.sample_infinite(trace_ctx, sampler.generate_1d()));
				auto& lt = *e_intr.divider->local_to_world;

				auto l_ctx = lt ^ rt ^ trace_ctx;
				METATRON_OPT_OR_CONTINUE(l_intr, (*e_intr.divider->light)(l_ctx));

				auto p_e = e_intr.pdf * l_intr.pdf;
				mis_e *= math::guarded_div(p_e, scatter_pdf);
				auto mis_w = math::guarded_div(1.f, spectra::avg(mis_s + mis_e));
				L_e += beta * mis_w * l_intr.L;
				continue;
			}

			auto& acc = acc_opt.value();
			auto& div = acc.divider;;
			auto& intr = acc.intr_opt.value();
			auto& lt = *div->local_to_world;
			if (scattered || crossed) {
				intr.p = rt | lt | math::expand(intr.p, 1.f);
				intr.n = math::normalize(rt | lt | intr.n);
				inside = math::dot(-trace_ctx.r.d, intr.n) < 0.f;
				intr.n *= inside ? -1.f : 1.f;
			}

			auto& mt = *medium_to_world;
			auto m_ctx = mt ^ rt ^ trace_ctx;
			METATRON_OPT_OR_CALLBACK(m_intr, medium->sample(m_ctx, intr.t, sampler.generate_1d()), {
				terminated = true;
				continue;
			});
			m_intr.p = rt | mt | math::expand(m_intr.p, 1.f);

			auto hit = m_intr.t >= intr.t;

			beta *= m_intr.transmittance / m_intr.pdf;
			mis_s *= m_intr.spectra_pdf / m_intr.pdf;
			mis_e *= m_intr.spectra_pdf / m_intr.pdf;

			if (!hit) {
				auto mis_a = math::guarded_div(1.f, spectra::avg(mis_s));
				L_e += mis_a * beta * m_intr.sigma_a * m_intr.L;

				auto p_a = math::guarded_div(m_intr.sigma_a.value.front(), m_intr.sigma_maj.value.front());
				auto p_s = math::guarded_div(m_intr.sigma_s.value.front(), m_intr.sigma_maj.value.front());
				auto p_n = math::guarded_div(m_intr.sigma_n.value.front(), m_intr.sigma_maj.value.front());

				auto u = sampler.generate_1d();
				auto mode = math::Discrete_Distribution{{p_a, p_s, p_n}}.sample(u);
				if (mode == 0uz) {
					beta *= m_intr.sigma_a / p_a;
					terminated = true;
				} else if (mode == 1uz) {
					phase = std::move(m_intr.phase);
					auto pt = math::Transform{{}, {1.f},
						math::Quaternion<f32>::from_rotation_between(-trace_ctx.r.d, {0.f, 1.f, 0.f})
					};

					auto p_ctx = pt | trace_ctx;
					METATRON_OPT_OR_CALLBACK(p_intr, phase->sample(p_ctx, sampler.generate_2d()), {
						terminated = true;
						continue;
					});
					p_intr.wi = pt ^ math::expand(p_intr.wi, 0.f);

					beta *= m_intr.sigma_s / p_s * p_intr.f / p_intr.pdf;
					mis_s *= m_intr.sigma_s / m_intr.sigma_maj / p_s;
					mis_e = mis_s;

					scattered = true;
					crossed = false;
					scatter_f = p_intr.f;
					scatter_pdf = p_intr.pdf;
					history_trace_ctx = trace_ctx;
					trace_ctx = {{m_intr.p, p_intr.wi}, {}, L_e, nullptr, phase.get()};
				} else {
					beta *= m_intr.sigma_n / p_n;
					mis_s *= (m_intr.sigma_n / m_intr.sigma_maj) / p_n;
					mis_e /= p_n;

					intr.t -= m_intr.t;
					trace_ctx.r.o = m_intr.p;
					scattered = false;
					crossed = false;
				}
				continue;
			}

			if (!rdiff.differentiable) {
				auto rd = ct ^ ddiff;
				auto st = math::Transform{{}, {1.f},
					math::Quaternion<f32>::from_rotation_between(rd.r.d, math::normalize(intr.p))
				};
				rdiff = st | rd;
			}
			rdiff.differentiable = false;
			
			auto ldiff = lt ^ rt ^ rdiff;
			auto tangent = shape::Plane{intr.p, intr.n};
			METATRON_OPT_OR_BREAK(d_intr, tangent(ldiff.r));
			METATRON_OPT_OR_BREAK(dx_intr, tangent(ldiff.rx));
			METATRON_OPT_OR_BREAK(dy_intr, tangent(ldiff.ry));
			
			auto dpdx = dx_intr.p - d_intr.p;
			auto dpdy = dy_intr.p - d_intr.p;
			auto dpduv =  math::transpose(math::Matrix<f32, 2, 3>{intr.dpdu, intr.dpdv});
			auto duvdx = math::least_squares(dpduv, dpdx);
			auto duvdy = math::least_squares(dpduv, dpdy);

			auto tcoord = texture::Coordinate{intr.uv, duvdx[0], duvdy[0], duvdx[1], duvdy[1]};
			METATRON_OPT_OR_CALLBACK(mat_intr, div->material->sample(trace_ctx, tcoord), {
				scattered = false;
				crossed = true;
				history_trace_ctx = trace_ctx;

				medium = !inside ? div->interior_medium : div->exterior_medium;
				medium_to_world = !inside ? div->interior_transform : div->exterior_transform;
				trace_ctx.r.o = intr.p - 0.001f * intr.n;
				inside = !inside;
				continue;
			});

			do {
				if (spectra::max(mat_intr.L) < math::epsilon<f32>) {
					break;
				}

				METATRON_OPT_OR_BREAK(e_intr, emitter(trace_ctx, {div->light, div->local_to_world}));
				auto& div = *e_intr.divider;
				auto& lt = *div.local_to_world;

				auto p_e = e_intr.pdf * intr.pdf;
				mis_e *= math::guarded_div(p_e, scatter_pdf);
				auto mis_w = math::guarded_div(1.f, spectra::avg(mis_s + mis_e));
				L_e += mis_w * beta * mat_intr.L;
			} while (false);

			bsdf = std::move(mat_intr.bsdf);
			auto bt = math::Transform{{}, {1.f},
				math::Quaternion<f32>::from_rotation_between(intr.n, {0.f, 1.f, 0.f})
			};
			auto uc = sampler.generate_1d();
			auto u = sampler.generate_2d();

			auto b_ctx = bt | trace_ctx;
			METATRON_OPT_OR_CALLBACK(b_intr, bsdf->sample(b_ctx, {uc, u[0], u[1]}), {
				terminated = true;
				continue;
			});

			if (-b_ctx.r.d[1] * b_intr.wi[1] < 0.f) {
				int_ctx.medium = !inside ? div->interior_medium : div->exterior_medium;
				int_ctx.medium_to_world = !inside ? div->interior_transform : div->exterior_transform;
			}

			b_intr.wi = math::normalize(bt ^ math::expand(b_intr.wi, 0.f));
			auto trace_n = (math::dot(b_intr.wi, intr.n) > 0.f ? 1.f : -1.f) * intr.n;
			auto trace_p = intr.p + 0.001f * trace_n;

			scattered = true;
			crossed = false;
			history_trace_ctx = trace_ctx;
			trace_ctx = {{trace_p, b_intr.wi}, trace_n, L_e, bsdf.get(), nullptr};
			beta *= b_intr.f * math::guarded_div(std::abs(math::dot(b_intr.wi, trace_n)), b_intr.pdf);
			mis_e = mis_s;
			scatter_f = b_intr.f;
			scatter_pdf = b_intr.pdf;
		}

		return L_e;
	}
}
