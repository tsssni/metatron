#include <metatron/render/light/atmosphere.hpp>
#include <metatron/core/math/cmath.hpp>
#include <metatron/core/math/integral.hpp>
#include <metatron/core/math/trigonometric.hpp>
#include <metatron/core/stl/concept.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/ranges.hpp>
#include <vector>

namespace mtt::render {

namespace {

template<typename T, typename U>
auto read(buf<T>& storage, std::vector<U>&& intermediate, std::string const& file) -> void {
    auto prefix = std::string{"atmosphere/"};
    auto postfix = std::string{".bin"};
    auto path = prefix + file + postfix;
    auto data = core::filesystem::find(path);

    auto f = std::ifstream{data, std::ios::binary};
    if (!f.is_open()) core::abort("{} not open", path);

    auto header = std::string(3, '\0');
    auto version = 0u;
    if (!f.read(header.data(), 3) || (header != "SKY" && header != "SUN"))
        core::abort("{} has wrong header {}", path, header);
    f.read((char*)&version, sizeof(version));

    auto dims = 0uz;
    f.read((char*)&dims, sizeof(dims));
    auto elems = 1uz;
    auto shape = std::vector<usize>(dims);
    for (auto& s : shape) {
        f.read((char*)&s, sizeof(s));
        if (s == 0uz) core::abort("{} has zero dimension", path);
        elems *= s;
    }

    intermediate.resize(elems);
    f.read((char*)intermediate.data(), intermediate.size() * sizeof(U));
    f.close();

    storage = elems;
    storage.map();
    std::ranges::transform(intermediate, storage.ptr, [](U x) { return (T)x; });
    storage.unmap();
}

}

buf<f32> Atmosphere_Light::sky_params_table;
buf<f32> Atmosphere_Light::sky_radiance_table;
buf<f32> Atmosphere_Light::sun_radiance_table;
buf<f32> Atmosphere_Light::sun_limb_table;
buf<f32> Atmosphere_Light::tgmm_table;

auto init_atmosphere() noexcept -> void {
    read(Atmosphere_Light::sky_params_table, std::vector<f64>{}, "sky-params");
    read(Atmosphere_Light::sky_radiance_table, std::vector<f64>{}, "sky-radiance");
    read(Atmosphere_Light::sun_radiance_table, std::vector<f64>{}, "sun-radiance");
    read(Atmosphere_Light::sun_limb_table, std::vector<f64>{}, "sun-limb");
    read(Atmosphere_Light::tgmm_table, std::vector<f32>{}, "tgmm");
}

namespace {

struct Sky_Context final {
    f32 const* data;
    f32* storage;
    usize length;
    f32 turbidity;
    f32 albedo;
    f32 eta;
};

[[mtt::kernel]] auto load_sky(Sky_Context& ctx, uv3 px) noexcept -> void {
    auto x = core::pow<1, 3>(ctx.eta / (core::pi * 0.5f));

    auto t_low = (i32)ctx.turbidity - 1;
    auto t_high = t_low + 1;
    auto t_alpha = ctx.turbidity - t_high;

    auto a_low = (i32)ctx.albedo;
    auto a_high = a_low + 1;
    auto a_alpha = ctx.albedo - a_low;

    auto albedo_size = ctx.length * sky_num_ctls;
    auto turbidity_size = albedo_size * atmo_num_albedo;

    auto bezier = [&](i32 t, i32 a) {
        auto value = 0.f;
        auto c = fv<6>{1, 5, 10, 10, 5, 1};
        auto offset = t * turbidity_size + a * albedo_size + px[0];
        for (auto i = 0; i < sky_num_ctls; ++i)
            value += ctx.data[offset + i * ctx.length] * c[i] * core::pow(x, i) * core::pow(1.f - x, 5 - i);
        return value;
    };
    auto b0 = core::lerp(bezier(t_low, a_low), bezier(t_high, a_low), t_alpha);
    auto b1 = core::lerp(bezier(t_low, a_high), bezier(t_high, a_high), t_alpha);
    auto b = core::lerp(b0, b1, a_alpha);
    ctx.storage[px[0]] = b;
}

struct Sun_Context final {
    Atmosphere_Light* light;
    f32 const* data;
    f32 const* limb;
    fv2 direction;
    f32 aperture;
    f32 temperature;
    f32 intensity;
};

[[mtt::kernel]] auto load_sun(Sun_Context& ctx, uv3 px) noexcept -> void {
    auto bspec = Blackbody_Spectrum{ctx.temperature};
    auto sun_scale = fv<atmo_num_lambda>{};
    for (auto i = 0; i < atmo_num_lambda; ++i)
        sun_scale[i] = bspec(atmo_lambda[i]) * sun_blackbody_scale / sun_perp_radiance[i];

    auto ratio = (core::sum(sun_scale) - sun_scale[0] - sun_scale[1]) / 9.f;
    auto lambda = (px[0] / sun_num_ctls) % atmo_num_lambda;
    auto scale = ctx.intensity / ratio * sun_scale[lambda];
    auto turbidity_size = sun_num_segments * atmo_num_lambda * sun_num_ctls;

    auto t_low = (i32)ctx.light->turbidity - 1;
    auto t_high = t_low + 1;
    auto t_alpha = ctx.light->turbidity - t_high;

    auto r_low = ctx.data[t_low * turbidity_size + px[0]];
    auto r_high = ctx.data[t_high * turbidity_size + px[0]];
    ctx.light->sun_radiance[px[0]] = core::lerp(r_low, r_high, t_alpha) * scale;

    if (px[0] < atmo_num_lambda * sun_num_limb_params)
        ctx.light->sun_limb[px[0]] = ctx.limb[px[0]];
    if (px[0] < atmo_num_lambda)
        ctx.light->sky_radiance[px[0]] *= ctx.intensity / ratio * sun_scale[px[0]];

    if (px[0] == 0) {
        ctx.light->cos_sun = core::cos(ctx.aperture * 0.5f);
        ctx.light->phi_sun = ctx.direction[1];
        ctx.light->area = (1.f - core::cos(sun_aperture * 0.5f)) / (1.f - ctx.light->cos_sun);
        ctx.light->sun_distr = core::Cone_Distribution{ctx.light->cos_sun};
    }
}

struct Tgmm_Context final {
    Atmosphere_Light* light;
    f32 const* data;
    f32 eta;
};

[[mtt::kernel]] auto load_tgmm(Tgmm_Context& ctx, uv3 px) noexcept -> void {
    auto t_idx = core::clamp(ctx.light->turbidity - 2.f, 0.f, (f32)(tgmm_num_turbility - 1));
    auto t_low = (i32)t_idx;
    auto t_high = core::min(t_low + 1, tgmm_num_turbility - 1);
    auto t_alpha = t_idx - t_low;

    auto eta_idx = core::clamp((core::degrees(ctx.eta) - 2.f) / 3.f, 0.f, (f32)(sun_num_segments - 1));
    auto eta_low = (i32)eta_idx;
    auto eta_high = core::min(eta_low + 1, sun_num_segments - 1);
    auto eta_alpha = eta_idx - eta_low;

    auto t_eta = im<4, 2>{
        {t_low, eta_low},
        {t_low, eta_high},
        {t_high, eta_low},
        {t_high, eta_high},
    };
    auto b_w = fv4{
        (1.f - t_alpha) * (1.f - eta_alpha),
        (1.f - t_alpha) * eta_alpha,
        t_alpha * (1.f - eta_alpha),
        t_alpha * eta_alpha,
    };

    auto offset = [&](u32 idx) {
        auto [t, eta] = t_eta[idx / tgmm_num_mixture];
        return ((t * tgmm_num_segments + eta) * tgmm_num_mixture + idx % tgmm_num_mixture) * (tgmm_num_gaussian_params + 1);
    };
    auto base = offset(px[0]);
    ctx.light->tgmm_phi_distr[px[0]].update(ctx.data[base], ctx.data[base + 2], 0.f, 2.f * core::pi);
    ctx.light->tgmm_theta_distr[px[0]].update(ctx.data[base + 1], ctx.data[base + 3], 0.f, 0.5f * core::pi);

    auto total = 0.f;
    auto prefix = 0.f;
    for (auto i = 0u; i < tgmm_num_gaussian; ++i) {
        auto weight = ctx.data[offset(i) + 4] * b_w[i / tgmm_num_mixture];
        total += weight;
        if (i <= px[0]) prefix += weight;
    }
    ctx.light->tgmm_distr.pdf[px[0]] = ctx.data[base + 4] * b_w[px[0] / tgmm_num_mixture] / total;
    ctx.light->tgmm_distr.cdf[px[0] + 1] = prefix;

    if (px[0] == 0) {
        ctx.light->tgmm_distr.count = tgmm_num_gaussian;
        ctx.light->tgmm_distr.cdf[0] = 0.f;
    }
}

auto constexpr integral_num_samples = 200u;

struct Integral_Context final {
    Atmosphere_Light* light;
    Spectrum CIE_Y;
    fv2 const* nodes;
    fv2* partials;
};

[[mtt::kernel]] auto integrate(Integral_Context& ctx, uv3 px, uv3 group, u32 wavesize) noexcept -> void {
    auto lane = px[0] % wavesize;
    auto row = ctx.nodes[group[0]];
    auto partial = fv2{0.f};

    for (auto i = lane; i < integral_num_samples; i += wavesize) {
        auto column = ctx.nodes[i];
        auto phi = core::pi * (column[0] + 1.f);
        auto cos_phi = core::cos(phi);
        auto sin_phi = core::sin(phi);

        auto cos_theta = 0.5f * (row[0] + 1.f);
        auto sin_theta = core::pow<1, 2>(1.f - core::pow<2>(cos_theta));
        auto wi = fv3{sin_theta * cos_phi, cos_theta, sin_theta * sin_phi};
        auto sky_state = Atmosphere_Light::State{cos_theta, core::dot(wi, ctx.light->d), ctx.light->cos_sun};

        auto cos_gamma = 0.5f * ((1.f - ctx.light->cos_sun) * row[0] + (1.f + ctx.light->cos_sun));
        auto sin_gamma = core::pow<1, 2>(1.f - core::pow<2>(cos_gamma));
        auto wi_sun = fv3{sin_gamma * cos_phi, cos_gamma, sin_gamma * sin_phi};
        wi_sun = core::normalize(fv3{ctx.light->t | core::expand(wi_sun, 0.f)});
        auto sun_state = Atmosphere_Light::State{core::unit_to_cos_theta(wi_sun), cos_gamma, ctx.light->cos_sun};

        auto luminance = fv2{0.f};
        for (auto j = 0; j < atmo_num_lambda; ++j) {
            auto weight = ctx.CIE_Y(atmo_lambda[j]);
            luminance[0] += ctx.light->hosek_sky(j, sky_state) * weight;
            luminance[1] += ctx.light->area * ctx.light->hosek_sun(j, sun_state) * ctx.light->hosek_limb(j, sun_state) * weight;
        }

        partial += luminance * fv2{core::pi * 0.5f, core::pi * 0.5f * (1.f - ctx.light->cos_sun)} * (row[1] * column[1]);
    }

    partial = {core::wave::sum(partial[0]), core::wave::sum(partial[1])};
    if (core::wave::elect()) ctx.partials[group[0]] = partial;
}

[[mtt::kernel]] auto reduce(Integral_Context& ctx, uv3 px, u32 wavesize) noexcept -> void {
    auto lane = px[0] % wavesize;
    auto luminance = fv2{0.f};
    for (auto i = lane; i < integral_num_samples; i += wavesize)
        luminance += ctx.partials[i];
    luminance = {core::wave::sum(luminance[0]), core::wave::sum(luminance[1])};
    if (core::wave::elect()) ctx.light->w_sky = core::guarded_div(luminance[0], luminance[0] + luminance[1]);
}

}

Atmosphere_Light::Atmosphere_Light(
    fv2 direction,
    f32 turbidity,
    f32 albedo,
    f32 aperture,
    f32 temperature,
    f32 intensity,
    Spectrum CIE_Y,
    core::recorder& recorder
) noexcept
: d(core::unit_spherical_to_cartesian(direction)), t(fq::from_rotation_between({0.f, 1.f, 0.f}, d)), turbidity(turbidity), albedo(albedo) {
    sky_params = atmo_num_lambda * sky_num_params;
    sky_radiance = atmo_num_lambda;
    sun_radiance = sun_num_segments * atmo_num_lambda * sun_num_ctls;
    sun_limb = atmo_num_lambda * sun_num_limb_params;

    tgmm_phi_distr = tgmm_num_gaussian;
    tgmm_theta_distr = tgmm_num_gaussian;
    tgmm_distr.count = tgmm_num_gaussian;
    tgmm_distr.pdf = tgmm_num_gaussian;
    tgmm_distr.cdf = tgmm_num_gaussian + 1;

    auto eta = core::pi * 0.5f - direction[0];
    auto params_ctx = Sky_Context{
        sky_params_table.ptr,
        sky_params.ptr,
        atmo_num_lambda * sky_num_params,
        turbidity,
        albedo,
        eta,
    };
    // core::pipenc{recorder, load_sky, params_ctx}.dispatch({atmo_num_lambda * sky_num_params, 1, 1});

    auto radiance_ctx = Sky_Context{
        sky_radiance_table.ptr,
        sky_radiance.ptr,
        atmo_num_lambda,
        turbidity,
        albedo,
        eta,
    };
    // core::pipenc{recorder, load_sky, radiance_ctx}.dispatch({atmo_num_lambda, 1, 1});

    auto sun_ctx = Sun_Context{
        this,
        sun_radiance_table.ptr,
        sun_limb_table.ptr,
        direction,
        aperture,
        temperature,
        intensity,
    };
    // core::pipenc{recorder, load_sun, sun_ctx}.dispatch({sun_num_segments * atmo_num_lambda * sun_num_ctls, 1, 1});

    auto tgmm_ctx = Tgmm_Context{this, tgmm_table.ptr, eta};
    // core::pipenc{recorder, load_tgmm, tgmm_ctx}.dispatch({tgmm_num_gaussian, 1, 1});

    auto [nodes, weights] = core::gauss_legendre<f32>(integral_num_samples);
    auto integral_nodes = buf<fv2>{integral_num_samples};
    auto integral_partials = buf<fv2>{integral_num_samples};
    integral_nodes.map();
    for (auto i = 0u; i < integral_num_samples; ++i)
        integral_nodes[i] = {nodes[i], weights[i]};
    integral_nodes.unmap();

    auto ctx = Integral_Context{this, CIE_Y, integral_nodes.ptr, integral_partials.ptr};
    // core::pipenc{recorder, integrate, ctx}.dispatch({integral_num_samples, 1, 1}, uv3{0});
    // core::pipenc{recorder, reduce, ctx}.dispatch({1, 1, 1}, uv3{0});
    recorder.retain(std::move(integral_nodes), std::move(integral_partials));
}

auto Atmosphere_Light::operator()(
    core::Ray const& r, fv4 const& lambda
) const noexcept -> Light_Interaction {
    auto wi = core::normalize(r.d);
    auto cos_theta = core::unit_to_cos_theta(wi);
    auto sin_theta = core::unit_to_sin_theta(wi);
    auto cos_gamma = core::dot(d, wi);

    // clamp theta to give invalid direction reasonable result
    auto [theta, phi] = core::cartesian_to_unit_spherical(r.d);
    theta = core::clamp(
        theta < core::pi * 0.5f ? theta : core::pi - theta,
        0.f, core::pi * 0.5f - 1e-2f
    );
    wi = core::unit_spherical_to_cartesian({theta, phi});

    auto s = State{core::unit_to_cos_theta(wi), core::dot(d, wi), cos_sun};
    auto L = visit([&](f32 lambda, usize i) { return hosek(lambda, s); }, lambda);

    auto tgmm_phi = phi + core::pi * 0.5f - phi_sun;
    auto sun_pdf = 0.f;
    auto sky_pdf = 0.f;
    for (auto i = 0; cos_theta >= 0.f && i < tgmm_num_gaussian; ++i)
        sky_pdf += tgmm_phi_distr[i].pdf(tgmm_phi) * tgmm_theta_distr[i].pdf(theta) * tgmm_distr.pdf[i];
    sky_pdf = core::guarded_div(sky_pdf, core::sin(theta));
    sun_pdf = cos_gamma >= cos_sun ? sun_distr.pdf() : 0.f;
    auto pdf = core::lerp(sun_pdf, sky_pdf, w_sky);

    return {
        .L = L,
        .wi = r.d,
        .p = r.o + r.d * 65504.f,
        .t = 65504.f,
        .pdf = pdf,
    };
}

auto Atmosphere_Light::sample(
    core::Ray_Context const& ctx, fv2 const& u
) const noexcept -> Light_Interaction {
    auto wi = fv3{};
    if (u[0] < w_sky) {
        auto idx = tgmm_distr.sample(u[0] / w_sky);
        auto u_phi = core::guarded_div(
            u[0] / w_sky - tgmm_distr.cdf[idx],
            tgmm_distr.cdf[idx + 1] - tgmm_distr.cdf[idx]
        );
        auto u_theta = u[1];

        // data fix sun to phi = pi / 2
        auto tgmm_phi = tgmm_phi_distr[idx].sample(u_phi);
        auto tgmm_theta = tgmm_theta_distr[idx].sample(u_theta);
        auto phi = tgmm_phi + phi_sun - core::pi * 0.5f;
        auto theta = core::clamp(tgmm_theta, 1e-2f, core::pi * 0.5f - 1e-2f);
        wi = core::unit_spherical_to_cartesian({theta, phi});
    } else {
        auto u_sun = fv2{(u[0] - w_sky) / (1.f - w_sky), u[1]};
        wi = core::normalize(fv3{t | core::expand(sun_distr.sample(u_sun), 0.f)});
    }
    return (*this)({ctx.r.o, wi}, ctx.lambda);
}

auto Atmosphere_Light::flags() const noexcept -> Light_Flags {
    return Light_Flags::inf;
}

Atmosphere_Light::State::State(f32 cos_theta, f32 cos_gamma, f32 cos_sun) noexcept
: cos_theta(cos_theta), cos_gamma(cos_gamma), gamma(core::acos(cos_gamma)), sqrt_cos_theta(core::pow<1, 2>(cos_theta)) {
    auto eta = core::pi * 0.5f - core::acos(cos_theta);
    segment = core::min(sun_num_segments - 1, i32(core::pow<1, 3>(eta / (core::pi * 0.5f)) * sun_num_segments));
    x = eta - core::pi * 0.5f * core::pow<3>(f32(segment) / f32(sun_num_segments));
    auto sin_gamma_sqr = 1.f - core::pow<2>(cos_gamma);
    cos_psi = core::pow<1, 2>(1.f - sin_gamma_sqr / (1.f - core::pow<2>(cos_sun)));
}

auto Atmosphere_Light::hosek(f32 lambda, State const& s) const noexcept -> f32 {
    if (lambda > atmo_lambda.back()) return 0.f;
    auto [low, high, alpha] = split(lambda);

    auto L = core::lerp(
        hosek_sky(low, s),
        hosek_sky(high, s),
        alpha
    );

    if (s.cos_gamma >= cos_sun) {
        auto sun = core::lerp(hosek_sun(low, s), hosek_sun(high, s), alpha);
        auto limb = core::lerp(hosek_limb(low, s), hosek_limb(high, s), alpha);
        L += area * sun * limb;
    }

    return L;
}

auto Atmosphere_Light::hosek_sky(i32 idx, State const& s) const noexcept -> f32 {
    auto [A, B, C, D, E, F, G, I, H] = *(fv<9> const*)&sky_params[idx * sky_num_params];
    auto chi = [](f32 g, f32 cos_alpha) -> f32 {
        return core::guarded_div(
            1.f + core::pow<2>(cos_alpha),
            core::pow<3, 2>(1.f + core::pow<2>(g) - 2.f * g * cos_alpha)
        );
    };
    auto c0 = 1.f + A * core::exp(B / (s.cos_theta + 0.01f));
    auto c1 = 0.f
        + C + D * core::exp(E * s.gamma)
        + F * core::pow<2>(s.cos_gamma)
        + G * chi(H, s.cos_gamma)
        + I * s.sqrt_cos_theta;
    return c0 * c1 * sky_radiance[idx] / CIE_Y_integral;
}

auto Atmosphere_Light::hosek_sun(i32 idx, State const& s) const noexcept -> f32 {
    auto L = 0.f;
    auto x_pow = 1.f;
    for (auto i = 0; i < sun_num_ctls; ++i) {
        L += sun_radiance[s.segment * atmo_num_lambda * sun_num_ctls + idx * sun_num_ctls + i] * x_pow;
        x_pow *= s.x;
    }
    return L / CIE_Y_integral;
}

auto Atmosphere_Light::hosek_limb(i32 idx, State const& s) const noexcept -> f32 {
    auto l = 0.f;
    auto psi_pow = 1.f;
    for (auto i = 0; i < sun_num_limb_params; ++i) {
        l += sun_limb[idx * sun_num_limb_params + i] * psi_pow;
        psi_pow *= s.cos_psi;
    }
    return l;
}

auto Atmosphere_Light::split(f32 lambda) const noexcept -> std::tuple<i32, i32, f32> {
    auto norm = (lambda - atmo_lambda.front()) / atmo_step;
    auto low = core::min(atmo_lambda.size() - 2, usize(norm));
    auto high = low + 1;
    auto alpha = norm - low;
    return {low, high, alpha};
}

}
