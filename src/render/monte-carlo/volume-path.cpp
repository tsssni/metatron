#include <metatron/render/monte-carlo/volume-path.hpp>
#include <metatron/resource/shape/plane.hpp>
#include <metatron/core/math/distribution/discrete.hpp>
#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/optional.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::monte_carlo {
	auto Volume_Path_Integrator::sample(
		Status initial_status,
		view<accel::Acceleration> accel,
		view<emitter::Emitter> emitter,
		view<math::Sampler> sampler
	) const noexcept -> std::optional<spectra::Stochastic_Spectrum> {
		auto lambda_u = sampler->generate_1d();
		auto emission = spectra::Stochastic_Spectrum{lambda_u};
		auto beta = emission; beta = 1.f;
		auto mis_s = emission; mis_s = 1.f;
		auto mis_e = emission; mis_e = 0.f;

		auto depth = 0uz;
		auto max_depth = initial_status.max_depth;
		auto crossed = true;
		auto terminated = false;

		auto scattered = false;
		auto scatter_pdf = 0.f;
		auto scatter_f = emission;

		auto bsdf = poly<bsdf::Bsdf>{};
		auto phase = poly<phase::Phase_Function>{};
		auto trace_ctx = eval::Context{};
		auto history_ctx = eval::Context{};
		trace_ctx.r = initial_status.ray_differential.r;
		trace_ctx.spec = emission;

		auto acc_opt = std::optional<accel::Interaction>{};
		auto medium = initial_status.medium;
		auto medium_to_world = initial_status.local_to_world;
		auto& rdiff = initial_status.ray_differential;
		auto& ddiff = initial_status.default_differential;
		auto& rt = *initial_status.world_to_render;
		auto& ct = *initial_status.render_to_camera;
		
		while (true) {
			depth += usize(scattered);
			if (terminated || depth >= max_depth) {
				break;
			}

			auto q = spectra::max(beta * math::guarded_div(1.f, spectra::avg(mis_s)));
			if (q < 1.f && depth > 1uz) {
				auto rr_u = sampler->generate_1d();
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
				MTT_OPT_OR_RETURN(e_intr, emitter->sample(direct_ctx, sampler->generate_1d()));
				auto& et = *e_intr.divider->local_to_world;

				auto l_ctx = et ^ rt ^ direct_ctx;
				MTT_OPT_OR_RETURN(l_intr, e_intr.divider->light->sample(l_ctx, sampler->generate_2d()));

				auto print = initial_status.pixel == math::Vector<usize, 2>{571, 643} && l_intr.wi[2] == -1.f;
				l_intr.p = rt | et | math::expand(l_intr.p, 1.f);
				l_intr.wi = math::normalize(rt | et | math::expand(l_intr.wi, 0.f));
				direct_ctx.r.d = l_intr.wi;
				auto p_e = e_intr.pdf * l_intr.pdf;
				if (math::abs(p_e) < math::epsilon<f32>) {
					return;
				}

				auto p_s = 0.f;
				auto f = spectra::Stochastic_Spectrum{};

				if (direct_ctx.n != math::Vector<f32, 3>{0.f}) {
					auto flip_n = math::dot(-history_ctx.r.d, direct_ctx.n) < 0.f ? -1.f : 1.f;
					auto t = math::Transform{math::Matrix<f32, 4, 4>{
						math::Quaternion<f32>::from_rotation_between(flip_n * direct_ctx.n, {0.f, 1.f, 0.f})
					}};
					auto wo = t | math::expand(history_ctx.r.d, 0.f);
					auto wi = t | math::expand(l_intr.wi, 0.f);
					MTT_OPT_OR_RETURN(b_intr, (*bsdf)(wo, wi));
					f = b_intr.f * math::abs(math::dot(l_intr.wi, direct_ctx.n));
					p_s = b_intr.pdf;
				} else {
					MTT_OPT_OR_RETURN(p_intr, (*phase)(history_ctx.r.d, l_intr.wi));
					f = p_intr.f;
					p_s = p_intr.pdf;
				}

				auto acc_opt = std::optional<accel::Interaction>{};
				auto termenated = false;
				auto crossed = true;

				auto dedium = medium;
				auto direct_to_world = medium_to_world;
				auto gamma = beta * (f / p_e) / (scatter_f / scatter_pdf);
				auto mis_d = mis_s * p_s / p_e * f32(!(e_intr.divider->light->flags() & light::Flags::delta));
				auto mis_l = mis_s;

				while (true) {
					if (termenated || l_intr.t < 0.001f) {
						break;
					}

					if (spectra::max(gamma * math::guarded_div(1.f, spectra::avg(mis_d + mis_l))) < 0.05f) {
						auto q = 0.25f;
						if (sampler->generate_1d() > q) {
							gamma = 0.f;
							termenated = true;
							continue;
						} else {
							gamma /= q;
						}
					}

					if (crossed) {
						acc_opt = (*accel)(direct_ctx.r, direct_ctx.n);
						if (!acc_opt) {
							termenated = true;
							continue;
						}
						auto& acc = acc_opt.value();

						if (!acc.intr_opt) {
							termenated = true;
							continue;
						}
						auto& intr = acc_opt->intr_opt.value();

						auto& div = *acc.divider;
						auto& lt = *div.local_to_world;
						intr.p = rt | lt | math::expand(intr.p, 1.f);
						intr.n = math::normalize(rt | lt | intr.n);
						direct_ctx.inside = math::dot(-direct_ctx.r.d, intr.n) < 0.f;
						dedium = direct_ctx.inside ? div.int_medium : div.ext_medium;
						direct_to_world = direct_ctx.inside ? div.int_to_world : div.ext_to_world;
						intr.n *= direct_ctx.inside ? -1.f : 1.f;

						auto close_to_light = math::length(intr.p - l_intr.p) < 0.001f;
						if (!close_to_light && !(div.material->bsdf->flags() & bsdf::Flags::interface)) {
							termenated = true;
							gamma = 0.f;
							continue;
						} else if (close_to_light) {
							auto rd = ct ^ ddiff;
							auto st = math::Transform{math::Matrix<f32, 4, 4>{
								math::Quaternion<f32>::from_rotation_between(rd.r.d, math::normalize(intr.p))
							}};
							rd = st | rd;

							auto ldiff = lt ^ rt ^ rd;
							auto tangent = shape::Plane{intr.p, intr.n};
							MTT_OPT_OR_CALLBACK(tcoord, texture::grad(ldiff, intr), {
								termenated = true; gamma = 0.f; continue;
							});
							MTT_OPT_OR_CALLBACK(mat_intr, div.material->sample(direct_ctx, tcoord), {
								termenated = true; gamma = 0.f; continue;
							});
							l_intr.L = mat_intr.emission;
						}
					}

					auto& acc = acc_opt.value();
					auto& div = acc.divider;
					auto& intr = acc.intr_opt.value();

					auto& mt = *direct_to_world;
					auto m_ctx = mt ^ rt ^ direct_ctx;
					MTT_OPT_OR_CALLBACK(m_intr, dedium->sample(m_ctx, intr.t, sampler->generate_1d()), {
						gamma = 0.f;
						termenated = true;
						break;
					});
					m_intr.p = rt | mt | math::expand(m_intr.p, 1.f);
					l_intr.t -= m_intr.t;

					auto hit = m_intr.t >= intr.t;

					gamma *= m_intr.transmittance / m_intr.pdf;
					mis_d *= m_intr.spectra_pdf / m_intr.pdf;
					mis_l *= m_intr.spectra_pdf / m_intr.pdf;
					if (print) {
						std::println("{}", gamma.value);
						std::println("{}", m_intr.transmittance.value);
					}

					if (!hit) {
						gamma *= m_intr.sigma_n;
						mis_d *= m_intr.sigma_n / m_intr.sigma_maj;
						intr.t -= m_intr.t;
						direct_ctx.r.o = m_intr.p;
						crossed = false;
					} else {
						direct_ctx.r.o = intr.p - 0.001f * intr.n;
						crossed = true;
					}
					continue;
				}

				auto mis_u = math::guarded_div(1.f, spectra::avg(mis_d + mis_l));
				emission += gamma * mis_u * l_intr.L;
			}();

			if (scattered || crossed) {
				acc_opt = (*accel)(trace_ctx.r, trace_ctx.n);
			}

			if (!acc_opt || !acc_opt.value().intr_opt) {
				terminated = true;

				MTT_OPT_OR_CONTINUE(e_intr, emitter->sample_infinite(trace_ctx, sampler->generate_1d()));
				auto& lt = *e_intr.divider->local_to_world;

				auto l_ctx = lt ^ rt ^ trace_ctx;
				MTT_OPT_OR_CONTINUE(l_intr, (*e_intr.divider->light)(l_ctx));

				auto p_e = e_intr.pdf * l_intr.pdf;
				mis_e *= math::guarded_div(p_e, scatter_pdf);
				auto mis_w = math::guarded_div(1.f, spectra::avg(mis_s + mis_e));
				emission += beta * mis_w * l_intr.L;
				continue;
			}

			auto& acc = acc_opt.value();
			auto& div = acc.divider;
			auto& intr = acc.intr_opt.value();
			auto& lt = *div->local_to_world;
			if (scattered || crossed) {
				intr.p = rt | lt | math::expand(intr.p, 1.f);
				intr.n = math::normalize(rt | lt | intr.n);
				trace_ctx.inside = math::dot(-trace_ctx.r.d, intr.n) < 0.f;
				medium = trace_ctx.inside ? div->int_medium : div->ext_medium;
				medium_to_world = trace_ctx.inside ? div->int_to_world : div->ext_to_world;

				auto flip_n = trace_ctx.inside ? -1.f : 1.f;
				intr.n *= flip_n;
				intr.tn = rt | lt | math::expand(intr.tn * flip_n, 0.f);
				intr.bn = rt | lt | math::expand(intr.bn * flip_n, 0.f);
			}

			auto& mt = *medium_to_world;
			auto m_ctx = mt ^ rt ^ trace_ctx;
			MTT_OPT_OR_CALLBACK(m_intr, medium->sample(m_ctx, intr.t, sampler->generate_1d()), {
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
				emission += mis_a * beta * m_intr.sigma_a * m_intr.sigma_e;

				auto p_a = math::guarded_div(m_intr.sigma_a.value[0], m_intr.sigma_maj.value[0]);
				auto p_s = math::guarded_div(m_intr.sigma_s.value[0], m_intr.sigma_maj.value[0]);
				auto p_n = math::guarded_div(m_intr.sigma_n.value[0], m_intr.sigma_maj.value[0]);

				auto u = sampler->generate_1d();
				auto mode = math::Discrete_Distribution{{p_a, p_s, p_n}}.sample(u);
				if (mode == 0uz) {
					beta *= m_intr.sigma_a / p_a;
					terminated = true;
				} else if (mode == 1uz) {
					phase = std::move(m_intr.phase);
					auto pt = math::Transform{math::Matrix<f32, 4, 4>{
						math::Quaternion<f32>::from_rotation_between(-trace_ctx.r.d, {0.f, 1.f, 0.f})
					}};

					auto p_ctx = pt | trace_ctx;
					MTT_OPT_OR_CALLBACK(p_intr, phase->sample(p_ctx, sampler->generate_2d()), {
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
					history_ctx = trace_ctx;
					trace_ctx = {{m_intr.p, p_intr.wi}, {}, emission};
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
				auto st = math::Transform{math::Matrix<f32, 4, 4>{
					math::Quaternion<f32>::from_rotation_between(rd.r.d, math::normalize(intr.p))
				}};
				rdiff = st | rd;
			}
			rdiff.differentiable = false;
			auto ldiff = lt ^ rt ^ rdiff;

			MTT_OPT_OR_BREAK(tcoord, texture::grad(ldiff, intr));
			MTT_OPT_OR_BREAK(mat_intr, div->material->sample(trace_ctx, tcoord));

			[&]() {
				if (spectra::max(mat_intr.emission) < math::epsilon<f32>) {
					return;
				}

				MTT_OPT_OR_RETURN(e_intr, (*emitter)(trace_ctx, {div->light, div->local_to_world}));
				auto& div = *e_intr.divider;
				auto& lt = *div.local_to_world;

				auto p_e = e_intr.pdf * intr.pdf;
				mis_e *= math::guarded_div(p_e, scatter_pdf);
				auto mis_w = math::guarded_div(1.f, spectra::avg(mis_s + mis_e));
				emission += mis_w * beta * mat_intr.emission;
			}();

			auto tbn = math::transpose(math::Matrix<f32, 3, 3>{intr.tn, intr.bn, intr.n});
			intr.n = tbn | mat_intr.normal;

			if (mat_intr.degraded && !spectra::coherent(emission)) {
				spectra::degrade(emission); spectra::degrade(beta);
				spectra::degrade(mis_s); spectra::degrade(mis_e);
				trace_ctx.spec = emission;
			}

			bsdf = std::move(mat_intr.bsdf);
			auto bt = math::Transform{math::Matrix<f32, 4, 4>{
				math::Quaternion<f32>::from_rotation_between(intr.n, {0.f, 1.f, 0.f})
			}};
			auto uc = sampler->generate_1d();
			auto u = sampler->generate_2d();

			auto b_ctx = bt | trace_ctx;
			MTT_OPT_OR_CALLBACK(b_intr, bsdf->sample(b_ctx, {uc, u[0], u[1]}), {
				terminated = true;
				continue;
			});

			if (b_ctx.r.d == b_intr.wi) {
				scattered = false;
				crossed = true;
				history_ctx = trace_ctx;
				trace_ctx.r.o = intr.p - 0.001f * intr.n;
				continue;
			}

			scattered = true;
			crossed = b_intr.wi[1] < 0.f;

			auto trace_n = (crossed ? -1.f : 1.f) * intr.n;
			auto trace_p = intr.p + 0.001f * trace_n;
			b_intr.wi = math::normalize(bt ^ math::expand(b_intr.wi, 0.f));
			b_intr.f *= math::abs(math::dot(b_intr.wi, trace_n));

			history_ctx = trace_ctx;
			trace_ctx = {{trace_p, b_intr.wi}, trace_n, emission};
			beta *= b_intr.f / b_intr.pdf;
			mis_e = mis_s;
			scatter_f = b_intr.f;
			scatter_pdf = b_intr.pdf;
		}

		return emission;
	}
}
