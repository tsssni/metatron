#include <metatron/render/monte-carlo/volume-path.hpp>
#include <metatron/core/math/distribution/discrete.hpp>
#include <metatron/core/math/quaternion.hpp>

namespace metatron::mc {
	auto Volume_Path_Integrator::sample(
		Ray ray,
		divider::Acceleration const& accel,
		light::Emitter const& emitter,
		math::Sampler const& sampler
	) const -> std::optional<std::unique_ptr<spectra::Stochastic_Spectrum>> {
		// TODO: add direct lighting

		auto Le = spectra::Stochastic_Spectrum{3uz, 0.f};
		auto beta = spectra::Stochastic_Spectrum{3uz, 0.f};
		beta.value = std::vector<f32>(3, 1.f);

		auto constexpr max_depth = 11uz;
		auto depth = 0uz;
		auto terminated = false;
		auto scattered = true;
		auto div_opt = std::optional<divider::Divider const*>{};
		auto intr_opt = std::optional<shape::Interaction>{};
		
		while (true) {
			depth += usize(scattered);
			if (terminated || depth > max_depth) {
				break;
			}

			auto rr_u = sampler.generate_1d();
			auto q = spectra::max(beta);
			if (q < 1.f && depth > 1.f) {
				if (rr_u > q) {
					break;
				} else {
					beta /= q;
				}
			}

			if (scattered) {
				div_opt = accel(ray.r);
				intr_opt = !div_opt
					? std::optional<shape::Interaction>{}
					: (*div_opt.value()->shape)(ray.r);
			}

			if (intr_opt) {
				auto& div = div_opt.value();
				auto& intr = intr_opt.value();

				if (ray.medium) {
					auto m_intr_opt = ray.medium->sample({&Le, ray.r, intr.t}, sampler.generate_1d());
					if (!m_intr_opt) {
						terminated = true;
						continue;
					}

					auto& m_intr = m_intr_opt.value();
					beta *= (*m_intr.transmittance) / m_intr.pdf;
					if (m_intr.t != intr.t) {
						// always add Le
						Le += beta * (*m_intr.sigma_a) * (*m_intr.Le);

						auto sigma_maj = *m_intr.sigma_a + *m_intr.sigma_s + *m_intr.sigma_n;
						auto p_a = m_intr.sigma_a->value[0] / sigma_maj.value[0];
						auto p_s = m_intr.sigma_s->value[0] / sigma_maj.value[0];
						auto p_n = m_intr.sigma_n->value[0] / sigma_maj.value[0];

						auto u = sampler.generate_1d();
						auto mode = math::Discrete_Distribution{std::array<f32, 3>{p_a, p_s, p_n}}.sample(u);
						if (mode == 0uz) {
							beta /= p_a;
							terminated = true;
						} else if (mode == 1uz) {
							auto p_intr_opt = m_intr.phase->sample({&Le, -ray.r.d}, sampler.generate_2d());
							if (!p_intr_opt) {
								terminated = true;
								continue;
							}

							auto& p_intr = p_intr_opt.value();
							ray.r.o = m_intr.p;
							ray.r.d = p_intr.wi;
							
							beta *= (*p_intr.f) / (p_s * p_intr.pdf);
							scattered = true;
						} else {
							intr.t -= m_intr.t;
							ray.r.o = m_intr.p;
							beta /= p_n;
							scattered = false;
						}
						continue;
					}
				}

				auto bsdf_opt = div->material->sample({&intr, &Le});
				if (!bsdf_opt) {
					terminated = true;
					continue;
				}
				
				auto& bsdf = bsdf_opt.value();
				auto render_to_local = math::Matrix<f32, 4, 4>{math::Quaternion<f32>::from_rotation_between(intr.n, {0.f, 1.f, 0.f})};
				auto local_to_render = math::inverse(render_to_local);
				auto uc = sampler.generate_1d();
				auto u = sampler.generate_2d();
				auto b_intr_opt = bsdf->sample({&Le, render_to_local | math::Vector<f32, 4>{ray.r.d}}, {uc, u[0], u[1]});
				if (!b_intr_opt) {
					terminated = true;
					continue;
				}

				auto& b_intr = b_intr_opt.value();
				b_intr.wi = local_to_render | math::Vector<f32, 4>{b_intr.wi};
				if (math::dot(-ray.r.d, b_intr.wi) < 0.f) {
					if (math::dot(b_intr.wi, intr.n) > 0.f) {
						ray.medium = div->exterior_medium;
					} else {
						ray.medium = div->interior_medium;
					}
				}

				ray.r.o = intr.p + 0.001f * b_intr.wi;
				ray.r.d = b_intr.wi;
				beta *= (*b_intr.f) / b_intr.pdf;
				scattered = true;
			} else {
				terminated = true;

				auto l_opt = emitter(ray.r);
				if (!l_opt) {
					continue;
				}

				auto l = l_opt.value();
				auto e_opt = (*l)(ray.r);
				if (!e_opt) {
					continue;
				}

				Le = beta * (Le & *e_opt.value());
			}
		}

		return std::make_unique<spectra::Stochastic_Spectrum>(Le);
	}
}
