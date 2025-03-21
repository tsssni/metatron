#include <metatron/render/monte-carlo/volume-path.hpp>
#include <metatron/core/math/distribution/discrete.hpp>
#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/optional.hpp>

namespace metatron::mc {
	auto Volume_Path_Integrator::sample(
		Ray ray,
		accel::Acceleration const& accel,
		emitter::Emitter const& emitter,
		math::Sampler const& sampler
	) const -> std::optional<spectra::Stochastic_Spectrum> {
		auto Le = spectra::Stochastic_Spectrum{3uz, 0.f};
		auto beta = spectra::Stochastic_Spectrum{3uz, 0.f};
		beta.value = std::vector<f32>(3, 1.f);

		auto constexpr max_depth = 11uz;
		auto depth = 0uz;
		auto terminated = false;

		auto scattered = false;
		auto scatter_pdf = 0.f;
		auto scatter_f = spectra::Stochastic_Spectrum{3uz, 0.f};
		auto scatter_ctx = eval::Context{};
		scatter_ctx.L = Le;

		auto div_opt = std::optional<accel::Divider const*>{};
		auto intr_opt = std::optional<shape::Interaction>{};
		auto bsdf = std::unique_ptr<material::Bsdf>{};
		auto phase = std::unique_ptr<phase::Phase_Function>{};
		
		while (true) {
			depth += usize(scattered);
			if (terminated || depth >= max_depth) {
				break;
			}

			auto q = spectra::max(beta);
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
					auto& t = *e_intr.divider->transform;
					scatter_ctx.p = t ^ math::Vector<f32, 4>{scatter_ctx.p, 1.f};
					scatter_ctx.n = t ^ math::Vector<f32, 4>{scatter_ctx.n, 0.f};

					OPTIONAL_OR_BREAK(l_intr, e_intr.divider->light->sample(scatter_ctx, sampler.generate_2d()));
					scatter_ctx.p = t | math::Vector<f32, 4>{scatter_ctx.p, 1.f};
					scatter_ctx.n = t | math::Vector<f32, 4>{scatter_ctx.n, 0.f};
					l_intr.wi = t | l_intr.wi;
					auto pl = e_intr.pdf * l_intr.pdf;

					auto l_r = math::Ray{scatter_ctx.p + l_intr.wi * 0.001f, l_intr.wi};
					auto d_div_opt = accel(l_r);
					if (d_div_opt.has_value()) {
						auto& d_div = d_div_opt.value();
						auto s = d_div->shape;
						auto s_intr_opt = (*s)(l_r);
						if (s_intr_opt.has_value() && d_div->material) {
							break;
						}
					}

					auto pb = 0.f;
					auto f = spectra::Stochastic_Spectrum{};
					if (bsdf) {
						auto t = hierarchy::Transform{};
						t.rotation = math::Quaternion<f32>::from_rotation_between(scatter_ctx.n, {0.f, 1.f, 0.f});
						auto wo = t | ray.r.d;
						auto wi = t | l_intr.wi;
						OPTIONAL_OR_BREAK(b_intr, (*bsdf)(wo, wi, Le));
						f = b_intr.f;
						pb = b_intr.pdf;
					} else if (phase) {
						OPTIONAL_OR_BREAK(p_intr, (*phase)(ray.r.d, l_intr.wi, Le));
						f = p_intr.f;
						pb = p_intr.pdf;
					} else {
						break;
					}

					auto mis_w = math::guarded_div(pl, pb + pl);
					Le += mis_w * beta * (f / pl) / (scatter_f / scatter_pdf) * l_intr.Le;
				} while (false);
				
				ray.r = scatter_ctx.r;
				ray.r.o += ray.r.d * 0.001f;
			}

			if (scattered || depth == 0uz) {
				div_opt = accel(ray.r);
				intr_opt = {};
				if (div_opt) {
					auto div = div_opt.value();
					auto r = math::Ray{
						*div->transform ^ math::Vector<f32, 4>{ray.r.o, 1.f},
						*div->transform ^ math::Vector<f32, 4>{ray.r.d, 0.f}
					};
					intr_opt = (*div->shape)({r});
				}
			}

			if (!intr_opt) {
				terminated = true;

				OPTIONAL_OR_CONTINUE(e_intr, emitter.sample_infinite(scatter_ctx, sampler.generate_1d()));
				auto wo = *e_intr.divider->transform ^ ray.r.d;
				auto n = math::Vector<f32, 3>{0.f};
				if (scattered && scatter_ctx.n != n) {
					n = *e_intr.divider->transform ^ scatter_ctx.n;
				}

				OPTIONAL_OR_CONTINUE(l_intr, (*e_intr.divider->light)(wo, n, Le));
				auto pl = e_intr.pdf * l_intr.pdf;

				auto mis_w = depth == 0uz ? 1.f : math::guarded_div(scatter_pdf, scatter_pdf + pl);
				Le = beta * mis_w * l_intr.Le;
				continue;
			}

			auto& div = div_opt.value();
			auto& intr = intr_opt.value();
			intr.p = *div->transform | math::Vector<f32, 4>{intr.p, 1.f};
			intr.n = *div->transform | math::Vector<f32, 4>{intr.p, 0.f};

			if (ray.medium) {
				scatter_ctx.r.o = *ray.medium_transform ^ math::Vector<f32, 4>{scatter_ctx.r.o, 1.f};
				scatter_ctx.r.d = *ray.medium_transform ^ math::Vector<f32, 4>{scatter_ctx.r.d, 0.f};
				OPTIONAL_OR_CALLBACK(m_intr, ray.medium->sample(scatter_ctx, intr.t, sampler.generate_1d()), {
					terminated = true;
					continue;
				});
				scatter_ctx.r.o = *ray.medium_transform | math::Vector<f32, 4>{scatter_ctx.r.o, 1.f};
				scatter_ctx.r.d = *ray.medium_transform | math::Vector<f32, 4>{scatter_ctx.r.d, 0.f};
				beta *= m_intr.transmittance / m_intr.pdf;

				if (m_intr.t != intr.t) {
					Le += beta * m_intr.sigma_a * m_intr.Le;

					auto sigma_maj = m_intr.sigma_a + m_intr.sigma_s + m_intr.sigma_n;
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
						OPTIONAL_OR_CALLBACK(p_intr, phase->sample(scatter_ctx, sampler.generate_2d()), {
							terminated = true;
							continue;
						});

						beta *= p_intr.f / (p_s * p_intr.pdf);
						scattered = true;
						scatter_f = p_intr.f;
						scatter_pdf = p_intr.pdf;
						scatter_ctx = {m_intr.p, {}, {}, {m_intr.p, p_intr.wi}, Le};
					} else {
						intr.t -= m_intr.t;
						ray.r.o = m_intr.p;
						beta /= p_n;
						scattered = false;
					}
					continue;
				}
			}
			
			scatter_ctx.uv = intr.uv;
			OPTIONAL_OR_CALLBACK(mat_intr, div->material->sample(scatter_ctx), {
				terminated = true;
				continue;
			});
			bsdf = std::move(mat_intr.bsdf);
			phase.reset();

			if (spectra::max(mat_intr.Le) > math::epsilon<f32>) {
				do {
					auto n = math::Vector<f32, 3>{0.f};
					if (scatter_ctx.n != n) {
						n = scatter_ctx.n;
					}
					OPTIONAL_OR_CONTINUE(e_intr, emitter(*div->Le));
					OPTIONAL_OR_CONTINUE(l_intr, (*e_intr.divider->light)(ray.r.d, n, Le));

					auto pl = e_intr.pdf * l_intr.pdf;
					auto pb = scatter_pdf;
					auto mis_w = math::guarded_div(scatter_pdf, scatter_pdf + pl);
					Le += mis_w * beta * mat_intr.Le;
				} while (false);
			}

			auto bt = hierarchy::Transform{};
			bt.rotation = math::Quaternion<f32>::from_rotation_between(intr.n, {0.f, 1.f, 0.f});
			auto uc = sampler.generate_1d();
			auto u = sampler.generate_2d();

			ray.r.d = bt | math::Vector<f32, 4>{scatter_ctx.r.d, 0.f};
			OPTIONAL_OR_CALLBACK(b_intr, bsdf->sample(scatter_ctx, {uc, u[0], u[1]}), {
				terminated = true;
				continue;
			});
			ray.r.d = bt ^ math::Vector<f32, 4>{scatter_ctx.r.d, 0.f};
			b_intr.wi = bt ^ math::Vector<f32, 4>{b_intr.wi};

			auto flip_n = 1.f;
			if (math::dot(-ray.r.d, b_intr.wi) < 0.f) {
				if (math::dot(b_intr.wi, intr.n) > 0.f) {
					ray.medium = div->exterior_medium;
					ray.medium_transform = div->exterior_transform;
				} else {
					ray.medium = div->interior_medium;
					ray.medium_transform = div->interior_transform;
					flip_n = -1.f;
				}
			}

			beta *= b_intr.f / b_intr.pdf;
			scattered = true;
			scatter_f = b_intr.f;
			scatter_pdf = b_intr.pdf;
			scatter_ctx = {intr.p, flip_n * intr.n, intr.uv, {intr.p, b_intr.wi}, Le};
		}

		return Le;
	}
}
