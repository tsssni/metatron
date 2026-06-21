#include <metatron/render/monte-carlo/restir.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/device/encoder/argument.hpp>
#include <metatron/device/encoder/pipeline.hpp>

namespace mtt::monte_carlo {
    Restir_Integrator::Restir_Integrator(cref<Descriptor> desc) noexcept:
    reuse_iterations(desc.reuse_iterations),
    reuse_confidence(desc.reuse_confidence),
    spatial_samples(desc.spatial_samples) {}

    auto Restir_Integrator::acquire(cref<Context> ctx, cref<Resources> res) noexcept -> void {
        auto pixels = math::prod(uzv2{ctx.film->image.size});
        pathes[0] = pixels;
        pathes[1] = pixels;
        if (!ctx.image) return;
        constants = make_desc<shader::Argument>({"metatron/render/monte-carlo/restir.constants"});
        integrate = make_desc<shader::Pipeline>({"metatron/render/monte-carlo/restir.wave",
        {constants.get(), res.resources.get(), res.textures.get(), res.grids.get()}});
        auto args = encoder::Argument_Encoder{ctx.render, constants.get()};
        args.bind("constants.image", *ctx.image);
        args.upload();
        args.submit();
    }

    auto Restir_Integrator::release() noexcept -> void {
        integrate.reset();
        constants.reset();
    }

    auto Restir_Integrator::trace(ref<Context> ctx) const noexcept -> void {
        auto ct = *math::proxy::Transform::entity("/hierarchy/camera/render");
        auto spp = ctx.film->spp;
        auto depth = ctx.film->depth;
        auto size = uzv2{ctx.film->image.size};
        auto& image = ctx.film->image;

        auto trace = [&](auto&& px) {
            auto sp = sampler::proxy::Sampler{ctx.sampler, {{}, px, size, ctx.sample_index, spp, 0, ctx.seed}};
            sp.start();
            auto fixel = ctx.film(ctx.filter, px, sp.generate_pixel_2d());
            MTT_OPT_OR_CALLBACK(s, photo::Camera{}.sample(
                ctx.lens, fixel.position, fixel.dxdy, sp.generate_2d()
            ), stl::abort("ray generation failed"););
            s.ray_differential = ct ^ s.ray_differential;
            s.default_differential = ct ^ s.default_differential;
            auto spec = spectra::Stochastic_Spectrum{sp.generate_1d()};

            auto r = Ray{
                ctx.accel, ctx.emitter,
                sp, spec.lambda,
                s.ray_differential,
                s.default_differential,
                ct, px, ctx.sample_index, depth,
            };
            MTT_OPT_OR_CALLBACK(Li, sample(r),
                stl::abort("invalid value appears in pixel {} sample {}", px, ctx.sample_index);
            );
            Li.value /= s.pdf;
            fixel = Li;
        };
        stl::scheduler::sync_parallel(uzv2{size}, trace);
    }

    auto Restir_Integrator::wave(ref<Context> ctx) const noexcept -> void {
        struct {
            accel::Acceleration accel;
            emitter::Emitter emitter;
            sampler::Sampler sampler;
            filter::Filter filter;
            photo::Lens lens;
            photo::proxy::Film film;
            math::Transform ct;
            u32 seed;
            u32 sample_index;
            u32 integrator;
        } entry{
            ctx.accel, ctx.emitter, ctx.sampler, ctx.filter, ctx.lens, ctx.film,
            *math::proxy::Transform::entity("/hierarchy/camera/render"),
            ctx.seed, ctx.sample_index,
            ctx.integrator,
        };
        auto args = encoder::Argument_Encoder{ctx.render, constants.get()};
        args.acquire("constants", entry);
        args.acquire("constants.image", *ctx.image);
        args.submit();
        auto threads = uv3{ctx.image->width, ctx.image->height, 1};
        auto group = uv3{8, 8, 1};
        auto pipeline = encoder::Pipeline_Encoder{ctx.render, integrate.get()};
        pipeline.bind();
        pipeline.dispatch(threads, group);
        pipeline.submit();
    }

    auto Restir_Integrator::sample(ref<Ray> r) const noexcept -> opt<spectra::Stochastic_Spectrum> {
        auto constexpr reconnection_t = 1e-2f;
        auto constexpr half_mask = (1 << 16) - 1;
        auto path = Path{
            .Li = {r.lambda, fv4{0.f}},
            .pixel = (r.pixel[0] & half_mask) | ((r.pixel[1] << 16)),
        };
        auto beta = fv4{1.f};

        auto depth = 0u;
        auto scattered = false;
        auto specular = false;
        auto connectable = false;

        auto p = 0.f;
        auto rr = 1.f;
        auto f = fv4{0.f};
        auto bsdf = bsdf::Bsdf{};

        auto trace_ctx = math::Context{};
        auto history_ctx = math::Context{};
        trace_ctx.r = r.ray_differential.r;
        trace_ctx.lambda = r.lambda;

        auto acc_opt = opt<accel::Interaction>{};
        auto& rdiff = r.ray_differential;
        auto& ddiff = r.default_differential;
        auto& ct = r.render_to_camera;

        auto direct_lighting = [&]() {
            if (!scattered || specular) return;

            auto direct_ctx = trace_ctx;
            auto u = fv4{r.sampler.generate_2d(), r.sampler.generate_2d()};
            MTT_OPT_OR_RETURN(e_intr, r.emitter.sample(direct_ctx, u[0]));
            auto et = e_intr.local_to_render;
            auto light = e_intr.light;
            auto l_ctx = et ^ direct_ctx;
            MTT_OPT_OR_RETURN(l_intr, light.sample(l_ctx, {u[1], u[2]}));

            auto e_pdf = e_intr.pdf;
            auto l_pdf = l_intr.pdf;
            auto p_e = e_pdf * l_pdf;
            if (math::abs(p_e) < math::epsilon<f32>) return;

            l_intr.p = et | math::expand(l_intr.p, 1.f);
            l_intr.wi = math::normalize(et | math::expand(l_intr.wi, 0.f));
            direct_ctx.r.d = l_intr.wi;

            auto flip_n = math::dot(-history_ctx.r.d, direct_ctx.n) < 0.f ? -1.f : 1.f;
            auto m = fm44{fq::from_rotation_between(flip_n * direct_ctx.n, {0.f, 1.f, 0.f})};
            auto wo = math::normalize(m | math::expand(history_ctx.r.d, 0.f));
            auto wi = math::normalize(m | math::expand(l_intr.wi, 0.f));
            MTT_OPT_OR_RETURN(b_intr, bsdf(wo, wi));
            auto g = b_intr.f * math::abs(math::dot(l_intr.wi, direct_ctx.n));
            auto q = b_intr.pdf;
            auto gamma = beta * (g / p_e) / (f / p);

            auto acc_opt = opt<accel::Interaction>{};
            auto mis_d = q / p_e * f32(!(e_intr.light.flags() & light::Flags::delta));
            auto mis_l = fv4{1.f};
            auto mis_u = math::guarded_div(1.f, math::avg(mis_d + mis_l));
            acc_opt = r.accel(direct_ctx.r, direct_ctx.n);
            if (!acc_opt || !acc_opt->intr_opt) {
                path({trace_ctx.lambda, mis_u * gamma * l_intr.L}, rr, u[3]);
                return;
            }

            auto& acc = *acc_opt;
            auto& intr = *acc.intr_opt;
            auto& div = *acc.divider;
            auto lt = div.local_to_render;

            intr.p = lt | math::expand(intr.p, 1.f);
            intr.n = math::normalize(lt | intr.n);
            direct_ctx.inside = math::dot(-direct_ctx.r.d, intr.n) < 0.f;
            intr.n *= direct_ctx.inside ? -1.f : 1.f;

            auto light_before_surface = l_intr.t < intr.t - 0.001f;
            auto close_to_light = math::length(intr.p - l_intr.p) < 0.001f;
            auto is_emissive = div.material.flags() & material::Flags::emissive;
            if (!light_before_surface && (!is_emissive || !close_to_light)) return;
            else if (!light_before_surface) {
                auto st = math::Transform{};
                st.transform = fm44{fq::from_rotation_between(ddiff.r.d, math::normalize(intr.p))};
                st.inv_transform = math::transpose(st.transform);
                auto rd = st | ddiff;

                auto ldiff = lt ^ rd;
                auto d_intr = intr;
                d_intr.p = lt ^ d_intr.p;
                d_intr.n = lt ^ d_intr.n;
                MTT_OPT_OR_RETURN(tcoord, texture::grad(ldiff, d_intr));
                MTT_OPT_OR_RETURN(mat_intr, div.material.sample(direct_ctx, tcoord));
                l_intr.L = mat_intr.emission;
            }
            path({trace_ctx.lambda, mis_u * gamma * l_intr.L}, rr, u[3]);
        };

        while (true) {
            depth += usize(scattered);
            if (depth >= r.max_depth) break;

            auto q = math::max(beta * rr);
            if (q < 1.f) {
                auto rr_u = r.sampler.generate_1d();
                if (rr_u > q) break;
                else rr /= q;
            }

            direct_lighting();
            acc_opt = r.accel(trace_ctx.r, trace_ctx.n);
            auto eu = r.sampler.generate_1d();
            if (!acc_opt || !acc_opt->intr_opt) {
                MTT_OPT_OR_BREAK(e_intr, r.emitter.sample_infinite(trace_ctx, r.sampler.generate_1d()));
                auto light = e_intr.light;
                auto lt = e_intr.local_to_render;

                auto l_ctx = lt ^ trace_ctx;
                MTT_OPT_OR_BREAK(l_intr, light(l_ctx.r, l_ctx.lambda));

                auto mis_e = fv4{specular ? 0.f : math::guarded_div(e_intr.pdf * l_intr.pdf, p)};
                auto mis_w = math::guarded_div(1.f, math::avg(1.f + mis_e));
                path({trace_ctx.lambda, beta * mis_w * l_intr.L}, rr, eu);
                break;
            }

            auto& acc = *acc_opt;
            auto& intr = *acc.intr_opt;
            auto& div = acc.divider;
            auto lt = div->local_to_render;

            intr.p = lt | math::expand(intr.p, 1.f);
            intr.n = math::normalize(lt | intr.n);
            trace_ctx.inside = math::dot(-trace_ctx.r.d, intr.n) < 0.f;

            auto flip_n = trace_ctx.inside ? -1.f : 1.f;
            intr.n *= flip_n;
            intr.tn = math::normalize(lt | math::expand(intr.tn * flip_n, 0.f));
            intr.bn = math::normalize(lt | math::expand(intr.bn * flip_n, 0.f));

            if (!rdiff.differentiable) {
                auto st = math::Transform{};
                st.transform = fm44{fq::from_rotation_between(ddiff.r.d, math::normalize(intr.p))};
                st.inv_transform = math::transpose(st.transform);
                rdiff = st | ddiff;
            }
            rdiff.differentiable = false;
            auto ldiff = lt ^ rdiff;
            auto l_intr = intr;
            l_intr.p = lt ^ l_intr.p;
            l_intr.n = lt ^ l_intr.n;
            MTT_OPT_OR_BREAK(tcoord, texture::grad(ldiff, l_intr));
            MTT_OPT_OR_BREAK(mat_intr, div->material.sample(trace_ctx, tcoord));

            if (math::max(mat_intr.emission) > math::epsilon<f32>) {
                // TODO: need correct way to fetch area light pdf on GPU, use 1.f now
                auto mis_e = fv4{specular ? 0.f : math::guarded_div(intr.pdf * 1.f, p)};
                auto mis_w = math::guarded_div(1.f, math::avg(1.f + mis_e));
                path({trace_ctx.lambda, beta * mis_w * mat_intr.emission}, rr, eu);
            }

            auto tbn = math::transpose(fm33{intr.tn, intr.bn, intr.n});
            intr.n = tbn | mat_intr.normal;

            if (mat_intr.degraded && !math::constant(trace_ctx.lambda)) {
                beta = fv4{beta[0]};
                trace_ctx.lambda = fv4{trace_ctx.lambda[0]};
                history_ctx.lambda = trace_ctx.lambda;
            }

            bsdf = std::move(mat_intr.bsdf);
            auto bt = math::Transform{};
            bt.transform = fm44{fq::from_rotation_between(intr.n, {0.f, 1.f, 0.f})};
            bt.inv_transform = math::transpose(bt.transform);
            auto uc = r.sampler.generate_1d();
            auto u = r.sampler.generate_2d();

            auto b_ctx = bt | trace_ctx;
            b_ctx.r.d = math::normalize(b_ctx.r.d);
            MTT_OPT_OR_BREAK(b_intr, bsdf.sample(b_ctx, {uc, u[0], u[1]}));

            scattered = true;
            specular = bsdf.flags() & bsdf::Flags::specular;
            auto trace_n = (b_intr.wi[1] < 0.f ? -1.f : 1.f) * intr.n;
            auto trace_p = intr.p + 0.001f * trace_n;
            b_intr.wi = math::normalize(bt ^ math::expand(b_intr.wi, 0.f));
            b_intr.f *= math::abs(math::dot(b_intr.wi, trace_n));

            beta *= b_intr.f / b_intr.pdf;
            history_ctx.r = trace_ctx.r;
            history_ctx.n = trace_ctx.n;
            trace_ctx.r = {trace_p, b_intr.wi};
            trace_ctx.n = trace_n;

            if (connectable && b_intr.connectable && intr.t > reconnection_t) {
                path.divider = div;
                path.beta = beta;
                path.p = intr.p;
                path.wi = b_intr.wi;
                path.pdf = {p, b_intr.pdf};
                path.depth = depth - 1;
            }

            connectable = (path.depth == 0) && b_intr.connectable;
            f = b_intr.f;
            p = b_intr.pdf;
        }

        // support set of different path depth is not intersected
        path.reservoir.M = 1.f;
        path.reservoir.finalize();
        path.Li.value *= path.reservoir.W;
        if (!math::isfinite(path.Li.value)) return {};
        return path.Li;
    }
}
