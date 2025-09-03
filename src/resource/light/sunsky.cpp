#include <metatron/resource/light/sunsky.hpp>
#include <metatron/resource/spectra/discrete.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/trigonometric.hpp>
#include <metatron/core/math/gaussian.hpp>
#include <metatron/core/math/integral/gauss-legendre.hpp>
#include <metatron/core/math/distribution/cone.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/optional.hpp>
#include <metatron/core/stl/print.hpp>
#include <ranges>
#include <fstream>
#include <cstring>
#include <algorithm>

namespace mtt::light {
	std::vector<f32> Sunsky_Light::sky_params_table;
	std::vector<f32> Sunsky_Light::sky_radiance_table;
	std::vector<f32> Sunsky_Light::sun_radiance_table;
	std::vector<f32> Sunsky_Light::sun_limb_table;
	std::vector<f32> Sunsky_Light::tgmm_table;

	Sunsky_Light::Sunsky_Light(
		math::Vector<f32, 2> direction,
		f32 turbidity,
		f32 albedo,
		f32 aperture
	) noexcept:
	d(math::unit_sphere_to_cartesion(direction)),
	t(math::Quaternion<f32>::from_rotation_between({0.f, 1.f, 0.f}, d)),
	turbidity(turbidity), 
	albedo(albedo),
	cos_sun(std::cos(aperture * 0.5)),
	area(1.f
	* (1.f - std::cos(sun_aperture * 0.5f))
	/ (1.f - cos_sun)) {
		auto bezier = [](std::vector<f32> const& data, usize block_size, usize offset, f32 x) -> std::vector<f32> {
			auto interpolated = std::vector<f32>(block_size, 0.f);
			auto c = std::array<f32, 6>{1, 5, 10, 10, 5, 1};
			for (auto i = 0; i < 6; i++) {
				auto start = offset + i * block_size;
				auto coeff = c[i] * std::pow(x, i) * std::pow(1.f - x, 5 - i);

				auto source = data | std::views::drop(start) | std::views::take(block_size);
				std::ranges::transform(
					source, interpolated, interpolated.begin(),
					[coeff](f32 val, f32 accum) { return accum + val * coeff;}
				);
			}
			return interpolated;
		};

		auto lerp = [](std::vector<f32> const& x, std::vector<f32> const& y, f32 alpha) -> std::vector<f32> {
			auto z = std::vector<f32>(x.size());
			std::ranges::transform(x, y, z.begin(), [alpha](f32 x, f32 y){return std::lerp(x, y, alpha);});
			return z;
		};

		auto eta = math::pi * 0.5f - direction[0];
		auto x = std::pow(eta / (math::pi * 0.5f), 1.f / 3.f);

		auto t_high = i32(turbidity);
		auto t_low = t_high - 1;
		auto t_alpha = turbidity - t_high;
		auto a_low = i32(albedo);
		auto a_high = a_low + 1;
		auto a_alpha = albedo - a_low;

		auto load_sky = [&](mut<f32> storage, std::vector<f32> const& data) -> void {
			auto size = data.size();
			auto turbidity_size = size / sunsky_num_turbility;
			auto albedo_size = turbidity_size / sunsky_num_albedo;
			auto bezier_size = albedo_size / sky_num_ctls;

			auto b00 = bezier(data, bezier_size, turbidity_size * t_low + albedo_size * a_low, x);
			auto b10 = bezier(data, bezier_size, turbidity_size * t_high + albedo_size * a_low, x);
			auto b01 = bezier(data, bezier_size, turbidity_size * t_low + albedo_size * a_high, x);
			auto b11 = bezier(data, bezier_size, turbidity_size * t_high + albedo_size * a_high, x);

			auto b0 = lerp(b00, b10, t_alpha);
			auto b1 = lerp(b01, b11, t_alpha);
			auto b = lerp(b0, b1, a_alpha);
			std::memcpy(storage, b.data(), b.size() * sizeof(f32));
		};

		auto load_sun = [&]() {
			auto& sun_table = *(math::Matrix<f32,
				sunsky_num_turbility, sun_num_segments, sunsky_num_lambda, sun_num_ctls
			>*)(sun_radiance_table.data());
			auto t0 = sun_table[t_low];
			auto t1 = sun_table[t_high];
			sun_radiance = t0 * (1.f - t_alpha) + t1 * t_alpha;
		};

		// https://github.com/mitsuba-renderer/mitsuba3/blob/master/include/mitsuba/render/sunsky.h
		// load tgmm of 4 interpolation sources and sample them
		auto load_tgmm = [&]() {
			auto t_idx = std::clamp(turbidity - 2.f, 0.f, f32(sunsky_num_turbility - 2));
			auto t_low = i32(t_idx);
			auto t_high = std::min(t_low + 1, sunsky_num_turbility - 2);
			auto t_alpha = t_idx - t_low;

			auto eta_deg = math::degree(eta);
			auto eta_idx = std::clamp((eta_deg - 2.f) / 3.f, 0.f, f32(sun_num_segments - 1));
			auto eta_low = i32(eta_idx);
			auto eta_high = std::min(eta_low + 1, sun_num_segments - 1);
			auto eta_alpha = eta_idx - eta_low;

			auto t_eta = std::views::cartesian_product(
				std::array<i32, 2>{t_low, t_high},
				std::array<i32, 2>{eta_low, eta_high}
			);
			auto b_w = std::views::cartesian_product(
				std::array<f32, 2>{1.f - t_alpha, t_alpha},
				std::array<f32, 2>{1.f - eta_alpha, eta_alpha}
			) | std::views::transform([](auto&& w) {
				auto [x, y] = w;
				return x * y;
			});
			auto& tgmm = *(math::Matrix<f32,
				tgmm_num_turbility, tgmm_num_segments, tgmm_num_mixture, tgmm_num_gaussian_params + 1
			>*)(tgmm_table.data());

			auto w = std::vector<f32>(tgmm_num_gaussian);
			for (auto i = 0; i < tgmm_num_bilinear; i++) {
				auto [t, eta] = t_eta[i];
				auto b = b_w[i];
				for (auto j = 0; j < tgmm_num_mixture; j++) {
					auto idx = i * tgmm_num_mixture + j;
					tgmm_gaussian[idx] = tgmm[t][eta][j];
					w[idx] = tgmm[t][eta][j][tgmm_num_gaussian_params - 1] * b;
				}
			}
			tgmm_distr = math::Discrete_Distribution{w};
		};

		load_sky(sky_params.data(), sky_params_table);
		load_sky(sky_radiance.data(), sky_radiance_table);
		load_sun();
		load_tgmm();

		w_sky = hosek_integral();
	}

	auto Sunsky_Light::init() noexcept -> void {
		auto read = []
		<typename T, typename U>
		(std::vector<T>& storage, std::vector<U>&& intermediate, std::string const& file) -> void {
			auto& fs = stl::filesystem::instance();
			auto prefix = std::string{"sunsky/"};
			auto postfix = std::string{".bin"};
			auto path = prefix + file + postfix;
			MTT_OPT_OR_CALLBACK(data, fs.find(path), {
				std::println("{} not found", path);
				std::abort();
			});

			auto f = std::ifstream{data, std::ios::binary};
			if (!f.is_open()) {
				std::println("{} not open", path);
				std::abort();
			}

			auto header = std::string(3, '\0');
			auto version = 0u;
			if (false
			|| !f.read(header.data(), 3)
			|| (true
			&& header != "SKY"
			&& header != "SUN")) {
				std::println("{} has wrong header {}", path, header);
				std::abort();
			}
			f.read(mut<char>(&version), sizeof(version));

			auto dims = 0uz;
			f.read(mut<char>(&dims), sizeof(dims));
			auto elems = 1uz;
			auto shape = std::vector<usize>(dims);
			for (auto& s: shape) {
				f.read(mut<char>(&s), sizeof(s));
				if (s == 0uz) {
					std::println("{} has zero dimension", path);
					std::abort();
				}
				elems *= s;
			}

			storage.resize(elems);
			intermediate.resize(elems);
			f.read(mut<char>(intermediate.data()), intermediate.size() * sizeof(U));
			f.close();
			std::ranges::transform(intermediate, storage.begin(), [](U x){return T(x);});
		};

		read(sky_params_table, std::vector<f64>{}, "sky-params");
		read(sky_radiance_table, std::vector<f64>{}, "sky-radiance");
		read(sun_radiance_table, std::vector<f64>{}, "sun-radiance");
		read(sun_limb_table, std::vector<f64>{}, "sun-limb");
		read(tgmm_table, std::vector<f32>{}, "tgmm");
	}

	auto Sunsky_Light::operator()(
		eval::Context const& ctx
	) const noexcept -> std::optional<Interaction> {
		auto cos_theta = math::unit_to_cos_theta(ctx.r.d);
		auto cos_gamma = math::dot(d, ctx.r.d);

		auto L = ctx.spec & spectra::Spectrum::spectra["zero"];
		L.value = math::foreach([&](f32 lambda, usize i) {
			return hosek(lambda, cos_theta, cos_gamma);
		}, L.lambda);

		return Interaction{L};
	}

	auto Sunsky_Light::sample(
		eval::Context const& ctx,
		math::Vector<f32, 2> const& u
	) const noexcept -> std::optional<Interaction> {
		auto intr = std::optional<Interaction>{};
		auto pdf = 0.f;
		if (u[0] < w_sky) {
			intr = sample_sky(ctx, {u[0] / w_sky, u[1]});
			pdf = w_sky;
		} else {
			intr = sample_sun(ctx, {(u[0] - w_sky) / (1.f - w_sky), u[1]});
			pdf = 1.f - w_sky;
		}

		if (intr) {
			intr.value().pdf *= pdf;
		}
		return intr;
	}

	auto Sunsky_Light::flags() const noexcept -> Flags {
		return Flags::inf;
	}

	auto Sunsky_Light::hosek(f32 lambda, f32 cos_theta, f32 cos_gamma) const noexcept -> f32 {
		if (cos_theta <= 0.f || lambda > sunsky_lambda.back()) {
			return 0.f;
		}

		auto norm = (lambda - sunsky_lambda.front()) / sunsky_step;
		auto low = std::min(sunsky_lambda.size() - 2, usize(norm));
		auto high = low + 1;
		auto alpha = norm - low;

		auto L = std::lerp(
			hosek_sky(low, cos_theta, cos_gamma),
			hosek_sky(high, cos_theta, cos_gamma),
			alpha
		);
		if (cos_gamma >= cos_sun) {
			auto sun = std::lerp(
				hosek_sun(low, cos_theta),
				hosek_sun(high, cos_theta),
				alpha
			);
			auto ld = std::lerp(
				hosek_limb(low, cos_gamma),
				hosek_limb(high, cos_gamma),
				alpha
			);
			L += sun * ld * area;
		}
		return L / spectra::CIE_Y_integral;
	}

	auto Sunsky_Light::hosek_sky(i32 idx, f32 cos_theta, f32 cos_gamma) const noexcept -> f32 {
		auto gamma = std::acos(cos_gamma);
		auto [A, B, C, D, E, F, G, I, H] = sky_params[idx];
		auto chi = [](f32 g, f32 cos_alpha) -> f32 {
			return math::guarded_div(
				1.f + math::sqr(cos_alpha),
				std::pow(1.f + math::sqr(g) - 2.f * g * cos_alpha, 1.5f)
			);
		};
		auto c0 = 1.f + A * std::exp(math::guarded_div(B, (cos_theta + 0.01f)));
		auto c1 = 0.f
		+ C + D * std::exp(E * gamma)
		+ F * math::sqr(cos_gamma)
		+ G * chi(H, cos_gamma)
		+ I * math::sqrt(cos_theta);
		auto val = c0 * c1 * sky_radiance[idx];
		return c0 * c1 * sky_radiance[idx];
	}

	auto Sunsky_Light::hosek_sun(i32 idx, f32 cos_theta) const noexcept -> f32 {
		auto eta = math::pi * 0.5f - std::acos(cos_theta);
		auto segment = std::min(
			sun_num_segments - 1,
			i32(std::pow(eta / (math::pi * 0.5f), 1.f / 3.f) * sun_num_segments)
		);
		auto x = eta - math::pi * 0.5f * math::pow(f32(segment) / f32(sun_num_segments), 3);
		auto L = 0.f;
		for (auto i = 0; i < sun_num_ctls; i++) {
			L += sun_radiance[segment][idx][i] * math::pow(x, i);
		}
		return L;
	}

	auto Sunsky_Light::hosek_limb(i32 idx, f32 cos_gamma) const noexcept -> f32 {
		auto& sun_limb = *(math::Matrix<f32, sunsky_num_lambda, sun_num_limb_params>*)(sun_limb_table.data());
		auto sin_gamma_sqr = 1.f - math::sqr(cos_gamma);
		auto cos_psi_sqr = 1.f - sin_gamma_sqr / (1.f - math::sqr(cos_sun));
		auto cos_psi = math::sqrt(cos_psi_sqr);
		auto l = 0.f;
		for (auto i = 0; i < sun_num_limb_params; i++) {
			l += sun_limb[idx][i] * math::pow(cos_psi, i);
		}
		return l;
	}


	auto Sunsky_Light::hosek_integral() const noexcept -> f32 {
		auto constexpr integral_num_samples = 200;
		auto [x, w] = math::gauss_legendre<f32>(integral_num_samples);
		auto cartesian_w = std::views::cartesian_product(w, w);

		// sky
		auto sky_luminance = [&]{
			auto J = math::pi * 0.5f;
			// [-1, 1] -> [0, 2pi]
			auto phi = x | std::views::transform([](auto x){return math::pi * (x + 1.f);});
			// [-1, 1] -> [0, 1]
			auto cos_theta = x | std::views::transform([](auto x){return 0.5f * (x + 1.f);});
			auto cos_theta_phi = std::views::cartesian_product(cos_theta, phi);
			auto cos_gamma = cos_theta_phi | std::views::transform([d = this->d](auto&& cos_theta_phi) {
				auto [cos_theta, phi] = cos_theta_phi;
				auto sin_theta = math::sqrt(1.f - math::sqr(cos_theta));
				auto cos_phi = std::cos(phi);
				auto sin_phi = std::sin(phi);
				auto wi = math::Vector<f32, 3>{sin_theta * cos_phi, cos_theta, sin_theta * sin_phi};
				return math::dot(wi, d);
			});

			auto luminance = 0.f;
			for (auto i = 0; i < sunsky_num_lambda; i++) {
				auto radiance = std::views::zip_transform([&](
					auto&& cos_theta_phi, auto cos_gamma, auto&& cartesian_w
				){
					auto [cos_theta, phi] = cos_theta_phi;
					auto [w_theta, w_gamma] = cartesian_w;
					return hosek_sky(i, cos_theta, cos_gamma) * w_theta * w_gamma;
				}, cos_theta_phi, cos_gamma, cartesian_w);
				auto integral = std::ranges::fold_left(radiance, 0.f, std::plus{}) * J;
				luminance += integral * (*spectra::Spectrum::spectra["CIE-Y"])(sunsky_lambda[i]);
			}
			return luminance;
		}();

		// sun
		auto sun_luminance = [&]{
			auto J = math::pi * 0.5f * (1.f - cos_sun);
			// [-1, 1] -> [0, 2pi]
			auto phi = x | std::views::transform([](auto x){return math::pi * (x + 1.f);});
			// [-1, 1] -> [cos_sun, 1]
			auto cos_gamma = x | std::views::transform([cos_sun = this->cos_sun](auto x){
				return 0.5f * ((1.f - cos_sun) * x + (1.f + cos_sun));
			});
			auto cos_gamma_phi = std::views::cartesian_product(cos_gamma, phi);
			auto cos_theta = cos_gamma_phi | std::views::transform([
				d = this->d,
				t = this->t
			](auto&& cos_gamma_phi) {
				auto [cos_gamma, phi] = cos_gamma_phi;
				auto sin_gamma = math::sqrt(1.f - math::sqr(cos_gamma));
				auto cos_phi = std::cos(phi);
				auto sin_phi = std::sin(phi);
				auto wi_sun = math::Vector<f32, 3>{sin_gamma * cos_phi, cos_gamma, sin_gamma * sin_phi};
				auto wi = math::normalize(math::Vector<f32, 3>{t | math::expand(wi_sun, 0.f)});
				return math::unit_to_cos_theta(wi);
			});

			auto luminance = 0.f;
			for (auto i = 0; i < sunsky_num_lambda; i++) {
				auto radiance = std::views::zip_transform([&](
					auto&& cos_gamma_phi, auto cos_theta, auto&& cartesian_w
				){
					auto [cos_gamma, phi] = cos_gamma_phi;
					auto [w_theta, w_gamma] = cartesian_w;
					return hosek_sun(i, cos_theta) * hosek_limb(i, cos_gamma) * w_theta * w_gamma;
				}, cos_gamma_phi, cos_theta, cartesian_w);
				auto integral = std::ranges::fold_left(radiance, 0.f, std::plus{}) * J;
				luminance += integral * (*spectra::Spectrum::spectra["CIE-Y"])(sunsky_lambda[i]);
			}
			return luminance * area;
		}();

		return math::guarded_div(sky_luminance, sky_luminance + sun_luminance);
	}

	auto Sunsky_Light::sample_sun(
		eval::Context const& ctx,
		math::Vector<f32, 2> const& u
	) const noexcept -> std::optional<Interaction> {
		auto intr = Interaction{};
		auto distr = math::Cone_Distribution{cos_sun};
		auto wi_gamma = distr.sample(u);
		auto wi_theta = math::normalize(math::Vector<f32, 3>{t | math::expand(distr.sample(u), 0.f)});
		auto cos_gamma = math::unit_to_cos_theta(wi_gamma);
		auto cos_theta = math::unit_to_cos_theta(wi_theta);

		auto L = ctx.spec;
		L.value = math::foreach([&](f32 lambda, auto i){
			auto norm = (lambda - sunsky_lambda.front()) / sunsky_step;
			auto low = std::min(sunsky_lambda.size() - 2, usize(norm));
			auto high = low + 1;
			auto alpha = norm - low;
			auto sun = std::lerp(
				hosek_sun(low, cos_theta),
				hosek_sun(high, cos_theta),
				alpha
			);
			auto ld = std::lerp(
				hosek_limb(low, cos_gamma),
				hosek_limb(high, cos_gamma),
				alpha
			);
			return sun * ld * area;
		}, L.lambda);

		return Interaction{
			.L = L,
			.wi = wi_theta,
			.p = ctx.r.o + wi_theta * 65504.f,
			.t = 65504.f,
			.pdf = distr.pdf(),
		};
	}

	auto Sunsky_Light::sample_sky(
		eval::Context const& ctx,
		math::Vector<f32, 2> const& u
	) const noexcept -> std::optional<Interaction> {
		auto wi = math::Vector<f32, 3>{};
		auto cos_gamma = math::dot(wi, d);
		auto cos_theta = math::dot(wi, {0.f, 1.f, 0.f});

		auto L = ctx.spec;
		L.value = math::foreach([&](f32 lambda, auto i){
			auto norm = (lambda - sunsky_lambda.front()) / sunsky_step;
			auto low = std::min(sunsky_lambda.size() - 2, usize(norm));
			auto high = low + 1;
			auto alpha = norm - low;
			return std::lerp(
				hosek_sky(low, cos_theta, cos_gamma),
				hosek_sky(high, cos_theta, cos_gamma),
				alpha
			);
		}, L.lambda);

		return Interaction{
			.L = L,
			.wi = wi,
			.p = ctx.r.o + wi * 65504.f,
			.t = 65504.f,
			.pdf = 1.f,
		};
	}
}
