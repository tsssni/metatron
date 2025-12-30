#include <metatron/resource/light/sunsky.hpp>
#include <metatron/resource/spectra/blackbody.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/trigonometric.hpp>
#include <metatron/core/math/gaussian.hpp>
#include <metatron/core/math/integral.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/ranges.hpp>
#include <metatron/core/stl/print.hpp>
#include <ranges>
#include <fstream>
#include <cstring>
#include <algorithm>

namespace mtt::light {
    buf<f32> Sunsky_Light::sky_params_table;
    buf<f32> Sunsky_Light::sky_radiance_table;
    buf<f32> Sunsky_Light::sun_radiance_table;
    buf<f32> Sunsky_Light::sun_limb_table;
    buf<f32> Sunsky_Light::tgmm_table;

    Sunsky_Light::Sunsky_Light(cref<Descriptor> desc) noexcept:
    d(math::unit_spherical_to_cartesian(desc.direction)),
    t(fq::from_rotation_between({0.f, 1.f, 0.f}, d)),
    turbidity(desc.turbidity), 
    albedo(desc.albedo) {
        auto bezier = [](buf<f32> data, usize block_size, usize offset, f32 x) -> std::vector<f32> {
            auto interpolated = std::vector<f32>(block_size, 0.f);
            auto c = std::array<f32, 6>{1, 5, 10, 10, 5, 1};
            for (auto i = 0; i < 6; ++i) {
                auto start = offset + i * block_size;
                auto coeff = c[i] * std::pow(x, i) * std::pow(1.f - x, 5 - i);
                auto source = data.subbuf(start, block_size);
                for (auto j = 0; j < block_size; ++j)
                    interpolated[j] += source[j] * coeff;
            }
            return interpolated;
        };

        auto lerp = [](cref<std::vector<f32>> x, cref<std::vector<f32>> y, f32 alpha) -> std::vector<f32> {
            auto z = std::vector<f32>(x.size());
            std::ranges::transform(x, y, z.begin(), [alpha](f32 x, f32 y){return math::lerp(x, y, alpha);});
            return z;
        };

        auto eta = math::pi * 0.5f - desc.direction[0];
        auto x = std::pow(eta / (math::pi * 0.5f), 1.f / 3.f);

        auto t_high = i32(turbidity);
        auto t_low = t_high - 1;
        auto t_alpha = turbidity - t_high;
        auto a_low = i32(albedo);
        auto a_high = a_low + 1;
        auto a_alpha = albedo - a_low;

        auto load_sky = [&](ref<buf<f32>> storage, buf<f32> data, usize length) -> void {
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
            storage = length;
            std::memcpy(storage.data(), b.data(), b.size() * sizeof(f32));
        };

        auto load_sun = [&]() {
            using Radiance_Table = fm<
                sunsky_num_turbility, sun_num_segments, sunsky_num_lambda, sun_num_ctls
            >;
            using Radiance = fm<sun_num_segments, sunsky_num_lambda, sun_num_ctls>;
            using Limb = fm<sunsky_num_lambda, sun_num_limb_params>;
            auto& sun_table = *mut<Radiance_Table>(sun_radiance_table.data());
            auto t0 = sun_table[t_low];
            auto t1 = sun_table[t_high];
            sun_radiance = sun_num_segments * sunsky_num_lambda * sun_num_ctls;
            sun_limb = sunsky_num_lambda * sun_num_limb_params;
            *mut<Radiance>(sun_radiance.data()) = t0 * (1.f - t_alpha) + t1 * t_alpha;
            *mut<Limb>(sun_limb.data()) = *mut<Limb>(sun_limb_table.data());

            auto bspec = spectra::Blackbody_Spectrum{desc.temperature};
            auto sun_scale = fv<sunsky_num_lambda>{};
            for (auto i = 0; i < sunsky_num_lambda; ++i) {
                auto lambda = sunsky_lambda[i];
                auto Lp = sun_perp_radiance[i];
                auto Lb = bspec(lambda) * sun_blackbody_scale;
                sun_scale[i] = Lb / Lp;
            }

            // only use visible spectra
            auto ratio = (math::sum(sun_scale) - sun_scale[0] - sun_scale[1]) / 9.f;
            cos_sun = std::cos(desc.aperture * 0.5f);
            phi_sun = desc.direction[1];
            area = (1.f - std::cos(sun_aperture * 0.5f)) / (1.f - cos_sun);
            sun_distr = math::Cone_Distribution{cos_sun};
            for (auto i = 0; i < sunsky_num_lambda; ++i)
                sky_radiance[i] *= desc.intensity / ratio * sun_scale[i];
            for (auto i = 0; i < sun_num_segments; ++i)
                for (auto j = 0; j < sunsky_num_lambda; ++j)
                    (*mut<Radiance>(sun_radiance.data()))[i][j] *= desc.intensity / ratio * sun_scale[j];
        };

        // https://github.com/mitsuba-renderer/mitsuba3/blob/master/include/mitsuba/render/sunsky.h
        // load tgmm of 4 interpolation sources and sample them
        auto load_tgmm = [&]() {
            auto t_idx = math::clamp(turbidity - 2.f, 0.f, f32(tgmm_num_turbility - 1));
            auto t_low = i32(t_idx);
            auto t_high = math::min(t_low + 1, tgmm_num_turbility - 1);
            auto t_alpha = t_idx - t_low;

            auto eta_deg = math::degrees(eta);
            auto eta_idx = math::clamp((eta_deg - 2.f) / 3.f, 0.f, f32(sun_num_segments - 1));
            auto eta_low = i32(eta_idx);
            auto eta_high = math::min(eta_low + 1, sun_num_segments - 1);
            auto eta_alpha = eta_idx - eta_low;

            auto t_eta = im<4, 2>{
                {t_low, eta_low}, {t_low, eta_high},
                {t_high, eta_low}, {t_high, eta_high},
            };
            auto b_w = fv4{
                (1.f - t_alpha) * (1.f - eta_alpha),
                (1.f - t_alpha) * eta_alpha,
                t_alpha * (1.f - eta_alpha),
                t_alpha * eta_alpha,
            };
            auto& tgmm = *(fm<
                tgmm_num_turbility, tgmm_num_segments, tgmm_num_mixture, tgmm_num_gaussian_params + 1
            >*)(tgmm_table.data());

            auto w = std::vector<f32>(tgmm_num_gaussian);
            tgmm_phi_distr = tgmm_num_gaussian;
            tgmm_theta_distr = tgmm_num_gaussian;
            for (auto i = 0; i < tgmm_num_bilinear; ++i) {
                auto [t, eta] = t_eta[i];
                auto b = b_w[i];
                for (auto j = 0; j < tgmm_num_mixture; ++j) {
                    auto idx = i * tgmm_num_mixture + j;
                    auto [mu_phi, mu_theta, sigma_phi, sigma_theta, w_g] = tgmm[t][eta][j];
                    tgmm_phi_distr[idx] = math::Truncated_Gaussian_Distribution{mu_phi, sigma_phi, 0.f, 2.f * math::pi};
                    tgmm_theta_distr[idx] = math::Truncated_Gaussian_Distribution{mu_theta, sigma_theta, 0.f, 0.5f * math::pi};
                    w[idx] = w_g * b;
                }
            }
            tgmm_distr = std::span<f32>{w};
        };

        load_sky(sky_params, sky_params_table, sunsky_num_lambda * sky_num_params);
        load_sky(sky_radiance, sky_radiance_table, sunsky_num_lambda);
        load_sun();
        load_tgmm();

        w_sky = hosek_integral();
    }

    auto Sunsky_Light::init() noexcept -> void {
        auto read = []
        <typename T, typename U>
        (ref<buf<T>> storage, rref<std::vector<U>> intermediate, cref<std::string> file) -> void {
            auto prefix = std::string{"sunsky/"};
            auto postfix = std::string{".bin"};
            auto path = prefix + file + postfix;
            auto data = stl::filesystem::find(path);

            auto f = std::ifstream{data, std::ios::binary};
            if (!f.is_open()) stl::abort("{} not open", path);

            auto header = std::string(3, '\0');
            auto version = 0u;
            if (false
            || !f.read(header.data(), 3)
            || (header != "SKY" && header != "SUN"))
                stl::abort("{} has wrong header {}", path, header);
            f.read(mut<char>(&version), sizeof(version));

            auto dims = 0uz;
            f.read(mut<char>(&dims), sizeof(dims));
            auto elems = 1uz;
            auto shape = std::vector<usize>(dims);
            for (auto& s: shape) {
                f.read(mut<char>(&s), sizeof(s));
                if (s == 0uz) stl::abort("{} has zero dimension", path);
                elems *= s;
            }

            intermediate.resize(elems);
            f.read(mut<char>(intermediate.data()), intermediate.size() * sizeof(U));
            f.close();

            if constexpr (std::is_same_v<T, U>) {
                storage = std::span{intermediate};
            } else {
                auto target = std::vector<T>(elems);
                std::ranges::transform(intermediate, target.begin(), [](U x){return T(x);});
                storage = std::span{target};
            }
        };

        read(sky_params_table, std::vector<f64>{}, "sky-params");
        read(sky_radiance_table, std::vector<f64>{}, "sky-radiance");
        read(sun_radiance_table, std::vector<f64>{}, "sun-radiance");
        read(sun_limb_table, std::vector<f64>{}, "sun-limb");
        read(tgmm_table, std::vector<f32>{}, "tgmm");
    }

    auto Sunsky_Light::operator()(
        cref<math::Ray> r, cref<fv4> lambda
    ) const noexcept -> opt<Interaction> {
        auto wi = math::normalize(r.d);
        auto cos_theta = math::unit_to_cos_theta(wi);
        auto sin_theta = math::unit_to_sin_theta(wi);
        auto cos_gamma = math::dot(d, wi);

        // clamp theta to give invalid direction reasonable result
        auto [theta, phi] = math::cartesian_to_unit_spherical(r.d);
        theta = math::clamp(
            theta < math::pi * 0.5f ? theta : math::pi - theta,
            0.f, math::pi * 0.5f - 1e-2f
        );
        wi = math::unit_spherical_to_cartesian({theta, phi});

        auto L = spectra::visit([&](f32 lambda, usize i) {
            return hosek(lambda, math::unit_to_cos_theta(wi), math::dot(d, wi));
        }, lambda);

        auto tgmm_phi = phi + math::pi * 0.5f - phi_sun;
        auto sun_pdf = 0.f;
        auto sky_pdf = 0.f;
        for (auto i = 0; cos_theta >= 0.f && i < tgmm_num_gaussian; ++i)
            sky_pdf += tgmm_phi_distr[i].pdf(tgmm_phi) * tgmm_theta_distr[i].pdf(theta) * tgmm_distr.pdf[i];
        sky_pdf = math::guarded_div(sky_pdf, std::sin(theta));
        sun_pdf = cos_gamma >= cos_sun ? sun_distr.pdf() : 0.f;
        auto pdf = math::lerp(sun_pdf, sky_pdf, w_sky);

        return Interaction{
            .L = L,
            .wi = r.d,
            .p = r.o + r.d * 65504.f,
            .t = 65504.f,
            .pdf = pdf,
        };
    }

    auto Sunsky_Light::sample(
        cref<math::Context> ctx, cref<fv2> u
    ) const noexcept -> opt<Interaction> {
        auto wi = fv3{};
        if (u[0] < w_sky) {
            auto idx = tgmm_distr.sample(u[0] / w_sky);
            auto u_phi = math::guarded_div(
                u[0] / w_sky - tgmm_distr.cdf[idx],
                tgmm_distr.cdf[idx + 1] - tgmm_distr.cdf[idx]
            );
            auto u_theta = u[1];

            // data fix sun to phi = pi / 2
            auto tgmm_phi = tgmm_phi_distr[idx].sample(u_phi);
            auto tgmm_theta = tgmm_theta_distr[idx].sample(u_theta);
            auto phi = tgmm_phi + phi_sun - math::pi * 0.5f;
            auto theta = math::clamp(tgmm_theta, 1e-2f, math::pi * 0.5f - 1e-2f);
            wi = math::unit_spherical_to_cartesian({theta, phi});
        } else {
            auto u_sun = fv2{(u[0] - w_sky) / (1.f - w_sky), u[1]};
            wi = math::normalize(fv3{t | math::expand(sun_distr.sample(u_sun), 0.f)});
        }
        return (*this)({ctx.r.o, wi}, ctx.lambda);
    }

    auto Sunsky_Light::flags() const noexcept -> Flags {
        return Flags::inf;
    }

    auto Sunsky_Light::hosek(f32 lambda, f32 cos_theta, f32 cos_gamma) const noexcept -> f32 {
        if (lambda > sunsky_lambda.back()) return 0.f;
        auto [low, high, alpha] = split(lambda);

        auto L = math::lerp(
            hosek_sky(low, cos_theta, cos_gamma),
            hosek_sky(high, cos_theta, cos_gamma),
            alpha
        );

        if (cos_gamma >= cos_sun) {
            L += area
            * math::lerp(
                hosek_sun(low, cos_theta),
                hosek_sun(high, cos_theta),
                alpha
            )
            * math::lerp(
                hosek_limb(low, cos_gamma),
                hosek_limb(high, cos_gamma),
                alpha
            );
        }

        return L;
    }

    auto Sunsky_Light::hosek_sky(i32 idx, f32 cos_theta, f32 cos_gamma) const noexcept -> f32 {
        auto gamma = math::acos(cos_gamma);
        auto [A, B, C, D, E, F, G, I, H] = *view<fv<9>>(&sky_params[idx * sky_num_params]);
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

        return c0 * c1 * sky_radiance[idx] / spectra::CIE_Y_integral;
    }

    auto Sunsky_Light::hosek_sun(i32 idx, f32 cos_theta) const noexcept -> f32 {
        auto eta = math::pi * 0.5f - math::acos(cos_theta);
        auto segment = math::min(
            sun_num_segments - 1,
            i32(std::pow(eta / (math::pi * 0.5f), 1.f / 3.f) * sun_num_segments)
        );
        auto x = eta - math::pi * 0.5f * math::pow(f32(segment) / f32(sun_num_segments), 3);
        auto L = 0.f;
        for (auto i = 0; i < sun_num_ctls; ++i)
            L += sun_radiance[
                segment * sunsky_num_lambda * sun_num_ctls + idx * sun_num_ctls + i
            ] * math::pow(x, i);
        return L / spectra::CIE_Y_integral;
    }

    auto Sunsky_Light::hosek_limb(i32 idx, f32 cos_gamma) const noexcept -> f32 {
        auto sin_gamma_sqr = 1.f - math::sqr(cos_gamma);
        auto cos_psi_sqr = 1.f - sin_gamma_sqr / (1.f - math::sqr(cos_sun));
        auto cos_psi = math::sqrt(cos_psi_sqr);
        auto l = 0.f;
        for (auto i = 0; i < sun_num_limb_params; ++i)
            l += sun_limb[idx * sun_num_limb_params + i] * math::pow(cos_psi, i);
        return l;
    }

    auto Sunsky_Light::hosek_integral() const noexcept -> f32 {
        auto constexpr integral_num_samples = 200;
        auto [x, w] = math::gauss_legendre<f32>(integral_num_samples);
        auto cartesian_w = stl::views::cartesian_product(w, w);

        auto sky_luminance = [&]{
            auto J = math::pi * 0.5f;
            // [-1, 1] -> [0, 2pi]
            auto phi = x | std::views::transform([](auto x){return math::pi * (x + 1.f);});
            // [-1, 1] -> [0, 1]
            auto cos_theta = x | std::views::transform([](auto x){return 0.5f * (x + 1.f);});
            auto cos_theta_phi = stl::views::cartesian_product(cos_theta, phi);
            auto cos_gamma = cos_theta_phi | std::views::transform([d = this->d](auto&& cos_theta_phi) {
                auto [cos_theta, phi] = cos_theta_phi;
                auto sin_theta = math::sqrt(1.f - math::sqr(cos_theta));
                auto cos_phi = std::cos(phi);
                auto sin_phi = std::sin(phi);
                auto wi = fv3{sin_theta * cos_phi, cos_theta, sin_theta * sin_phi};
                return math::dot(wi, d);
            });

            auto luminance = 0.f;
            for (auto i = 0; i < sunsky_num_lambda; ++i) {
                auto radiance = std::views::zip(cos_theta_phi, cos_gamma, cartesian_w)
                | std::views::transform([&](auto&& zipped) {
                    auto [cos_theta_phi, cos_gamma, cartesian_w] = zipped;
                    auto [cos_theta, phi] = cos_theta_phi;
                    auto [w_theta, w_gamma] = cartesian_w;
                    return hosek_sky(i, cos_theta, cos_gamma) * w_theta * w_gamma;
                });
                auto integral = std::ranges::fold_left(radiance, 0.f, std::plus{}) * J;
                auto CIE_Y = entity<spectra::Spectrum>("/spectrum/CIE-Y").data();
                luminance += integral * (*CIE_Y)(sunsky_lambda[i]);
            }
            return luminance;
        }();

        auto sun_luminance = [&]{
            auto J = math::pi * 0.5f * (1.f - cos_sun);
            // [-1, 1] -> [0, 2pi]
            auto phi = x | std::views::transform([](auto x){return math::pi * (x + 1.f);});
            // [-1, 1] -> [cos_sun, 1]
            auto cos_gamma = x | std::views::transform([cos_sun = this->cos_sun](auto x){
                return 0.5f * ((1.f - cos_sun) * x + (1.f + cos_sun));
            });
            auto cos_gamma_phi = stl::views::cartesian_product(cos_gamma, phi);
            auto cos_theta = cos_gamma_phi | std::views::transform([
                d = this->d,
                t = this->t
            ](auto&& cos_gamma_phi) {
                auto [cos_gamma, phi] = cos_gamma_phi;
                auto sin_gamma = math::sqrt(1.f - math::sqr(cos_gamma));
                auto cos_phi = std::cos(phi);
                auto sin_phi = std::sin(phi);
                auto wi_sun = fv3{sin_gamma * cos_phi, cos_gamma, sin_gamma * sin_phi};
                auto wi = math::normalize(fv3{t | math::expand(wi_sun, 0.f)});
                return math::unit_to_cos_theta(wi);
            });

            auto luminance = 0.f;
            for (auto i = 0; i < sunsky_num_lambda; ++i) {
                auto radiance = std::views::zip(cos_gamma_phi, cos_theta, cartesian_w)
                | std::views::transform([&](auto&& zipped) {
                    auto [cos_gamma_phi, cos_theta, cartesian_w] = zipped;
                    auto [cos_gamma, phi] = cos_gamma_phi;
                    auto [w_theta, w_gamma] = cartesian_w;
                    return area * hosek_sun(i, cos_theta) * hosek_limb(i, cos_gamma) * w_theta * w_gamma;
                });
                auto integral = std::ranges::fold_left(radiance, 0.f, std::plus{}) * J;
                auto CIE_Y = entity<spectra::Spectrum>("/spectrum/CIE-Y").data();
                luminance += integral * (*CIE_Y)(sunsky_lambda[i]);
            }
            return luminance;
        }();

        return math::guarded_div(sky_luminance, sky_luminance + sun_luminance);
    }

    auto Sunsky_Light::split(f32 lambda) const noexcept -> std::tuple<i32, i32, f32> {
        auto norm = (lambda - sunsky_lambda.front()) / sunsky_step;
        auto low = math::min(sunsky_lambda.size() - 2, usize(norm));
        auto high = low + 1;
        auto alpha = norm - low;
        return {low, high, alpha};
    }
}
