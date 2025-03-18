#include <metatron/render/monte-carlo/volume-path.hpp>
#include <metatron/core/math/distribution/discrete.hpp>
#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/optional.hpp>

namespace metatron::mc {
	auto Volume_Path_Integrator::sample(
		Ray ray,
		divider::Acceleration const& accel,
		light::Emitter const& emitter,
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
		auto scatter_ctx = eval::Context{&ray.r.o, {}, {}, &ray.r, &Le};

		auto div_opt = std::optional<divider::Divider const*>{};
		auto intr_opt = std::optional<shape::Interaction>{};
		
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
					OPTIONAL_BREAK(e_intr, emitter.sample(scatter_ctx, sampler.generate_2d()));
					OPTIONAL_BREAK(l_intr, e_intr.light->sample(scatter_ctx, sampler.generate_2d()));

					auto pl = e_intr.pdf * l_intr.pdf;
					auto pb = scatter_pdf;
					auto mis_w = math::guarded_div(pl, pb + pl);
					Le += mis_w * beta * scatter_pdf / pl * l_intr.Le;
				} while (false);
				
			}

			if (scattered || depth == 0uz) {
				div_opt = accel(ray.r);
				intr_opt = !div_opt
					? std::optional<shape::Interaction>{}
					: (*div_opt.value()->shape)(ray.r);
			}

			if (!intr_opt) {
				terminated = true;

				auto const& n = scattered && scatter_ctx.n ? *scatter_ctx.n : math::Vector<f32, 3>{0.f};
				OPTIONAL_CONTINUE(e_intr, emitter.sample_infinite(scatter_ctx, sampler.generate_1d()));
				OPTIONAL_CONTINUE(l_intr, (*e_intr.light)(ray.r.d, n, Le));

				auto pl = e_intr.pdf * l_intr.pdf;
				auto mis_w = scattered ? math::guarded_div(scatter_pdf, scatter_pdf + pl) : 1.f;
				beta /= e_intr.pdf;
				Le = beta * mis_w * (Le & l_intr.Le);
				continue;
			}

			auto& div = div_opt.value();
			auto& intr = intr_opt.value();

			if (ray.medium) {
				auto m_intr_opt = ray.medium->sample(scatter_ctx, intr.t, sampler.generate_1d());
				if (!m_intr_opt) {
					terminated = true;
					continue;
				}

				auto& m_intr = m_intr_opt.value();
				beta *= m_intr.transmittance / m_intr.pdf;
				if (m_intr.t != intr.t) {
					// always add Le
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
						OPTIONAL_CONTINUE_CALLBACK(p_intr, m_intr.phase->sample({{}, {}, {}, &ray.r, &Le}, sampler.generate_2d()), [&terminated]{terminated=true;});

						ray.r.o = m_intr.p;
						ray.r.d = p_intr.wi;
						
						beta *= p_intr.f / (p_s * p_intr.pdf);
						scattered = true;
						scatter_pdf = p_intr.pdf;
						scatter_ctx.n = {};
						scatter_ctx.uv = {};
					} else {
						intr.t -= m_intr.t;
						ray.r.o = m_intr.p;
						beta /= p_n;
						scattered = false;
					}
					continue;
				}
			}
			
			scatter_ctx = {&intr.p, &intr.n, &intr.uv, &ray.r, &Le};
			OPTIONAL_CONTINUE_CALLBACK(mat_intr, div->material->sample(scatter_ctx), [&terminated]{terminated=true;});
			auto& bsdf = mat_intr.bsdf;

			if (spectra::max(mat_intr.Le) > math::epsilon<f32>) {
				do {
					auto const& n = scattered && scatter_ctx.n ? *scatter_ctx.n : math::Vector<f32, 3>{0.f};
					OPTIONAL_CONTINUE(e_intr, emitter(*div->area_light));
					OPTIONAL_CONTINUE(l_intr, (*e_intr.light)(ray.r.d, n, Le));

					auto pl = e_intr.pdf * l_intr.pdf;
					auto pb = scatter_pdf;
					auto mis_w = scattered ? math::guarded_div(scatter_pdf, scatter_pdf + pl) : 1.f;
					Le += mis_w * beta * mat_intr.Le;
				} while (false);
			}

			auto render_to_local = math::Matrix<f32, 4, 4>{math::Quaternion<f32>::from_rotation_between(intr.n, {0.f, 1.f, 0.f})};
			auto local_to_render = math::inverse(render_to_local);
			auto uc = sampler.generate_1d();
			auto u = sampler.generate_2d();

			OPTIONAL_CONTINUE_CALLBACK(b_intr, bsdf->sample(scatter_ctx, {uc, u[0], u[1]}), [&terminated]{terminated=true;});
			ray.r.d = render_to_local | math::Vector<f32, 4>{ray.r.d};
			b_intr.wi = local_to_render | math::Vector<f32, 4>{b_intr.wi};

			if (math::dot(-ray.r.d, b_intr.wi) < 0.f) {
				if (math::dot(b_intr.wi, intr.n) > 0.f) {
					ray.medium = div->exterior_medium;
				} else {
					ray.medium = div->interior_medium;
				}
			}

			ray.r.o = intr.p;
			ray.r.d = b_intr.wi;
			beta *= (*b_intr.f) / b_intr.pdf;

			scattered = true;
			scatter_pdf = b_intr.pdf;
			scatter_ctx.n = &intr.n;
			scatter_ctx.uv = &intr.uv;
		}

		return Le;
	}
}
