#include <metatron/render/material/physical.hpp>
#include <metatron/render/material/bsdf.hpp>
#include <metatron/core/distribution/sphere.hpp>
#include <metatron/core/stl/concept.hpp>
#include <metatron/core/stl/print.hpp>
#include <metatron/core/stl/thread.hpp>

namespace mtt::render {

auto constexpr reconnection_alpha = 0.04f;

auto Physical_Bsdf::operator()(
    fv3 const& wo, fv3 const& wi, f32 u
) const noexcept -> Bsdf_Interaction {
    auto flags = this->flags();
    auto specular = flags & Bsdf_Flags::specular;
    if (false
        || (type != Type::plastic && specular)
        || core::abs(wo[1]) < core::epsilon<f32>
        || core::abs(wi[1]) < core::epsilon<f32>) return {};

    auto reflective = -wo[1] * wi[1] > 0.f;
    auto forward = wi[1] > 0.f;
    auto wm = core::normalize(reflective ? -wo + wi : -wo + wi * eta[0]);
    if (wm[1] < 0.f) wm *= -1.f;
    if (false
        || (type != Type::dielectric && (!reflective || !forward))
        || core::abs(wm[1]) < core::epsilon<f32>
        || core::dot(-wo, wm) < 0.f
        || core::dot((reflective ? 1.f : -1.f) * wi, wm) < 0.f) return {};

    if (type == Type::lambertian) return {
        .f = lambert(reflectance),
        .eta = eta,
        .wi = wi,
        .pdf = core::Cosine_Hemisphere_Distribution{}.pdf(core::unit_to_cos_theta(wi)),
    };

    auto replay = u != -1;
    auto R = Bsdf_Interaction{.f = fv4{0}, .eta = eta, .wi = wi, .pdf = 0};
    auto Fo = type != Type::plastic ? fv4{1.f} : fresnel(core::unit_to_cos_theta(-wo), eta, k);

    if (type == Type::plastic && (!replay || u >= Fo[0])) {
        auto Fi = fresnel(core::unit_to_cos_theta(wi), eta, k);
        auto pdf = core::Cosine_Hemisphere_Distribution{}.pdf(core::unit_to_cos_theta(wi));
        auto internal = 1.f
            * (1.f - Fi) * (1.f - Fo) / (core::pi * core::pow<2>(eta))
            * (reflectance / (1.f - reflectance * fresnel_reflectance));
        R.f = internal;
        R.pdf = (1.f - Fo[0]) * pdf;
        if (replay) return R;
    }
    if (replay && (alpha[0] < reconnection_alpha || alpha[1] < reconnection_alpha)) return {};

    if (type != Type::plastic || !specular) {
        auto F = fresnel(core::dot(-wo, wm), eta, k);
        auto D = trowbridge_reitz(wm, alpha[0], alpha[1]);
        auto G = smith_shadow(wo, wi, alpha[0], alpha[1]);

        auto pr = F[0];
        auto pt = 1.f - F[0];
        pr *= bool(flags & Bsdf_Flags::reflective);
        pt *= bool(flags & Bsdf_Flags::transmissive);
        if (pr == 0.f && pt == 0.f) return {};

        auto T = torrance_sparrow(
            reflective, pr, pt,
            F, D, G,
            wo, wi, wm,
            eta, alpha[0], alpha[1]
        );
        R.f += T.f;
        R.pdf += Fo[0] * T.pdf;
    }

    return R;
}

auto Physical_Bsdf::sample(
    core::Ray_Context const& ctx, fv3 const& u
) const noexcept -> Bsdf_Interaction {
    auto wo = ctx.r.d;
    auto flags = this->flags();
    auto specular = bool(flags & Bsdf_Flags::specular);
    auto Fo = (!specular && type != Type::plastic) ? fv4{0.f} : fresnel(core::unit_to_cos_theta(-wo), eta, k);

    if (specular && (type != Type::plastic || u[0] < Fo[0])) {
        auto wm = fv3{0.f, 1.f, 0.f};
        auto cos_theta_o = core::unit_to_cos_theta(-wo);
        if (core::abs(wo[1]) < core::epsilon<f32>) return {};

        auto pr = Fo[0] * bool(flags & Bsdf_Flags::reflective);
        auto pt = (1.f - Fo[0]) * bool(flags & Bsdf_Flags::transmissive);
        if (pr == 0.f && pt == 0.f) return {};

        auto reflective = u[0] < pr / (pr + pt);
        auto wi = reflective ? core::reflect(wo, wm) : core::refract(wo, wm, eta[0]);
        auto cos_theta_i = core::unit_to_cos_theta(wi);
        if (core::abs(wi[1]) < core::epsilon<f32>) return {};

        auto pdf = (reflective ? pr : pt) / (pr + pt);
        auto f = (reflective ? Fo : (1.f - Fo) / core::pow<2>(eta[0])) / core::abs(cos_theta_i);
        return {f, eta, wi, pdf};
    } else if (type == Type::dielectric || type == Type::conductive || (type == Type::plastic && u[0] < Fo[0])) {
        if (core::abs(wo[1]) < core::epsilon<f32>) return {};

        auto wy = core::normalize(-wo * fv3{alpha[0], 1.f, alpha[1]});
        auto wx = wy[1] < 1.f - core::epsilon<f32> ? core::cross(wy, fv3{0.f, 1.f, 0.f}) : fv3{1.f, 0.f, 0.f};
        auto wz = core::cross(wx, wy);

        // use polar disk distribution to fetch more samples near center
        auto distr = core::Polar_Disk_Distribution{};
        auto sample_p = distr.sample({u[1], u[2]});
        auto sample_h = core::pow<1, 2>(1.f - core::pow<2>(sample_p[0]));
        sample_p[1] = (1.f + wy[1]) / 2.f * sample_p[1] + (1.f - wy[1]) * sample_h / 2.f;

        auto sample_y = core::pow<1, 2>(1.f - core::dot(sample_p, sample_p));
        auto wm = sample_p[0] * wx + sample_y * wy + sample_p[1] * wz;
        if (core::abs(wm[1]) < core::epsilon<f32> || core::dot(-wo, wm) < 0.f) return {};
        // normal transformation with inverse transposed matrix
        wm = core::normalize(wm * fv3{alpha[0], 1.f, alpha[1]});

        auto F = fresnel(core::dot(-wo, wm), eta, k);
        auto D = trowbridge_reitz(wm, alpha[0], alpha[1]);

        auto pr = F[0] * bool(flags & Bsdf_Flags::reflective);
        auto pt = (1.f - F[0]) * bool(flags & Bsdf_Flags::transmissive);
        if (pr == 0.f && pt == 0.f) return {};

        auto reflective = (type == Type::plastic || type == Type::conductive) ? true : u[0] < pr / (pr + pt);
        auto wi = reflective ? core::reflect(wo, wm) : core::refract(wo, wm, eta[0]);
        auto G = smith_shadow(wo, wi, alpha[0], alpha[1]);
        if (core::abs(wi[1]) < core::epsilon<f32>) return {};

        auto R = torrance_sparrow(
            reflective, pr, pt,
            F, D, G,
            wo, wi, wm,
            eta, alpha[0], alpha[1]
        );
        R.pdf *= type == Type::plastic ? Fo[0] : 1.f;
        return R;
    } else {
        auto distr = core::Cosine_Hemisphere_Distribution{};
        auto wi = distr.sample({u[1], u[2]});
        auto pdf = distr.pdf(wi[1]);
        if (core::abs(wi[1]) < core::epsilon<f32>) return {};

        if (type == Type::lambertian) {
            auto f = lambert(reflectance);
            return {f, eta, wi, pdf, true};
        } else {
            auto Fi = fresnel(core::unit_to_cos_theta(wi), eta, k);
            auto f = 1.f
                * (1.f - Fi) * (1.f - Fo) / (core::pi * core::pow<2>(eta))
                * (reflectance / (1.f - reflectance * fresnel_reflectance));
            pdf *= (1.f - Fo[0]);
            return {f, eta, wi, pdf, true};
        }
    }
}

auto Physical_Bsdf::flags() const noexcept -> Bsdf_Flags {
    auto flags = 0;
    if (type == Type::lambertian || type == Type::plastic || type == Type::conductive) flags |= Bsdf_Flags::reflective;
    else if (type == Type::dielectric && eta == fv4{1.f}) flags |= Bsdf_Flags::transmissive;
    else flags |= Bsdf_Flags::transmissive | Bsdf_Flags::reflective;
    if (type != Type::lambertian && (alpha[0] == 0.f || alpha[1] == 0.f)) flags |= Bsdf_Flags::specular;
    return Bsdf_Flags(flags);
}

buf<f32> Physical_Material::fresnel_reflectance_table;

auto constexpr fresnel_num_samples = 65536;
auto constexpr fresnel_length = 256;

auto init_physical_material() noexcept -> void {
    Physical_Material::fresnel_reflectance_table = fresnel_length + 1;
    Physical_Material::fresnel_reflectance_table.map();
    core::scheduler::sync_parallel(uzv1{fresnel_length + 1}, [&](auto idx) {
        auto i = idx[0];
        auto integral = 0.0;
        auto eta = i * 3.f / fresnel_length;
        auto f0 = render::fresnel(0.f, eta, 0.f);
        for (auto i = 1; i <= fresnel_num_samples; ++i) {
            auto cos_theta_2 = f32(i) / f32(fresnel_num_samples);
            auto cos_theta = core::pow<1, 2>(cos_theta_2);
            auto f1 = render::fresnel(cos_theta, eta, 0.f);
            integral += (f0 + f1) * 0.5f / fresnel_num_samples;
        }
        Physical_Material::fresnel_reflectance_table[i] = integral;
    });
    Physical_Material::fresnel_reflectance_table.unmap();
}

auto Physical_Material::sample(
    core::Ray_Context const& ctx,
    fv2 const& uv, fv4 const& duvdxy
) const noexcept -> Material_Interaction<render::Physical_Bsdf> {
    auto guarded_sample = [&]<typename T>(T tex, auto const& fallback) {
        if (!tex) return fallback;
        if constexpr (std::same_as<T, render::Spectrum_Texture>)
            return tex(uv, duvdxy, ctx.lambda);
        else return tex(uv, duvdxy);
    };
    auto reflectance = guarded_sample(this->reflectance, fv4{2.f});
    auto eta = guarded_sample(this->eta, fv4{0.f});
    auto k = guarded_sample(this->k, fv4{0.f});
    auto emission = guarded_sample(this->emission, fv4{0.f});

    auto alpha = fv2{guarded_sample(this->alpha, fv4{0.f})};
    auto normal = guarded_sample(this->normal, fv4{0.5f, 0.5f, 1.f, 0.f});

    auto lambda = ctx.lambda;
    auto degraded = true
        && reflectance != core::saturate(reflectance)
        && eta != fv4{0.f} && !core::constant(eta)
        && k == fv4{0.f};

    if (degraded) {
        lambda = fv4{lambda[0]};
        reflectance = fv4{reflectance[0]};
        eta = fv4{eta[0]};
        k = fv4{k[0]};
        emission = fv4{emission[0]};
    }

    if (ctx.inside && eta != fv4{0.f}) eta = 1.f / eta;
    using Type = render::Physical_Bsdf::Type;
    auto type = Type{};
    auto based = reflectance == core::saturate(reflectance);
    auto surface = eta != fv4{0.f};
    auto conductive = k != fv4{0.f};

    if (based && !surface && !conductive) type = Type::lambertian;
    else if (!based && surface && !conductive) type = Type::dielectric;
    else if (!based && surface && conductive) type = Type::conductive;
    else if (based && surface && !conductive) type = Type::plastic;
    else {
        core::print("bsdf not physically possible with these attributes:");
        core::print("reflectance: {}", reflectance);
        core::print("eta: {}", eta);
        core::print("k: {}", k);
        core::abort();
    }

    auto reflect = [](auto eta, auto) {
        auto idx = eta / 3.f * fresnel_length;
        auto low = i32(idx);
        auto high = low + 1;
        auto alpha = idx - low;
        return core::lerp(fresnel_reflectance_table[low], fresnel_reflectance_table[high], alpha);
    };
    auto fresnel_reflectance = type == Type::plastic ? fv4{0.f} : core::foreach(reflect, eta);

    return Material_Interaction<render::Physical_Bsdf>{
        .bsdf = render::Physical_Bsdf{
            .reflectance = reflectance,
            .fresnel_reflectance = fresnel_reflectance,
            .eta = eta,
            .k = k,
            .alpha = alpha,
            .type = type,
        },
        .emission = emission,
        .normal = core::shrink(normal) * 2.f - 1.f,
        .degraded = degraded,
    };
}

auto Physical_Material::flags() const noexcept -> Material_Flags {
    return emission ? Material_Flags::emissive : Material_Flags(0);
}

}
