#include <metatron/render/monte-carlo/restir.hpp>
#include <metatron/core/stl/thread.hpp>
#include <metatron/device/encoder/argument.hpp>
#include <metatron/device/encoder/pipeline.hpp>
#include <metatron/device/encoder/transfer.hpp>
#include <metatron/device/opaque/buffer.hpp>

namespace mtt::monte_carlo {
    auto constexpr reconnection_t = 1e-2f;
    auto constexpr half_bits = 16;
    auto constexpr half_mask = (1 << half_bits) - 1;

    auto Restir_Integrator::Path::estimate(spectra::Stochastic_Spectrum Li, f32 W, f32 u) noexcept -> void {
        auto y = math::max(Li.value);
        auto r = math::Reservoir{
            .p_hat = y,
            .M = 0.f,
            .W = W,
        };
        r.shift(1.f, 1.f);
        if (this->r.merge(r, u)) this->Li = Li;
    }

    auto Restir_Integrator::Path::merge(cref<Path> p, f32 u) noexcept -> void {
        if (r.merge(p.r, u)) {
            auto r = this->r;
            *this = p;
            this->r = r;
        }
    }

    Restir_Integrator::Restir_Integrator(cref<Descriptor> desc) noexcept:
    reuse_iterations(desc.reuse_iterations),
    spatial_samples(desc.spatial_samples),
    spatial_radius(desc.spatial_radius) {}

    auto Restir_Integrator::upload(ref<Context> ctx) noexcept -> void {
        auto pixels = math::prod(uzv2{ctx.film->image.size});
        pathes[0] = pixels;
        pathes[1] = pixels;
    }

    auto Restir_Integrator::acquire(ref<Context> ctx, cref<Resources> res) noexcept -> void {
        if (!ctx.image) return;
        constants = make_obj<Constants>(Constants{
            ctx.accel, ctx.emitter, ctx.sampler, ctx.filter, ctx.lens, ctx.film,
            *math::proxy::Transform::entity("/hierarchy/camera/render"),
            ctx.seed, ctx.sample_index, 0,
            ctx.integrator, *ctx.image
        });
        arguments = make_desc<shader::Argument>({"metatron/render/monte-carlo/restir.constants"});

        auto pipeline = [&](std::string_view name) {
            return make_desc<shader::Pipeline>({name,
            {arguments.get(), res.resources.get(), res.textures.get(), res.grids.get()}});
        };
        integrate = pipeline("metatron/render/monte-carlo/restir.trace");
        restir = pipeline("metatron/render/monte-carlo/restir.reuse");

        auto args = encoder::Argument_Encoder{ctx.render, arguments.get()};
        args.push(*constants, {0, sizeof(Constants)});
        args.submit();
    }

    auto Restir_Integrator::release() noexcept -> void {
        integrate.reset();
        restir.reset();
        arguments.reset();
    }

    auto Restir_Integrator::trace(ref<Context> ctx) noexcept -> void {
        auto ct = *math::proxy::Transform::entity("/hierarchy/camera/render");
        auto spp = ctx.film->spp;
        auto depth = ctx.film->depth;
        auto size = uzv2{ctx.film->image.size};
        auto& image = ctx.film->image;
        auto iter = 0;

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
                ct, px, image.size,
                ctx.sample_index, depth,
            };
            MTT_OPT_OR_CALLBACK(p, sample(r),
                stl::abort("invalid value appears in pixel {} sample {}", px, ctx.sample_index);
            );
            if (reuse_iterations == 0) {
                p.Li.value *= p.r.W / s.pdf;
                fixel = p.Li;
            } else pathes[0][px[1] * size[0] + px[0]] = p;
        };

        auto reuse = [&](auto&& px) {
            auto sc = sampler::Context{{}, px, size, ctx.sample_index, spp, 0, ctx.seed};
            auto sp = sampler::proxy::Sampler{ctx.sampler, sc};
            sp.start();
            auto fixel = ctx.film(ctx.filter, px, sp.generate_pixel_2d());
            MTT_OPT_OR_CALLBACK(s, photo::Camera{}.sample(
                ctx.lens, fixel.position, fixel.dxdy, sp.generate_2d()
            ), stl::abort("ray generation failed"););
            s.ray_differential = ct ^ s.ray_differential;
            s.default_differential = ct ^ s.default_differential;

            auto pi = px[1] * size[0] + px[0];
            auto p = pathes[iter & 0x1][pi];
            auto dst = p; dst.r = {};
            auto canonical = 1.f;
            auto valid = 0;
            auto N = f32(spatial_samples);

            for (auto i = 0; i < spatial_samples; i++) {
                // reuse for rotation and ris
                sp.ctx.pixel = px;
                sp.ctx.dim = (spatial_samples * 4 + 1) * iter + i * 4;
                sp.start();
                auto u = fv4{sp.generate_2d(), sp.generate_2d()};
                auto theta = u[0] * math::pi * 2.f;
                auto py = iv2{px} + iv2{fv2{std::cosf(theta), std::sinf(theta)} * spatial_radius};
                if (math::clamp(py, iv2{0}, iv2{size - 1}) != py) continue;

                auto ni = py[1] * size[0] + py[0];
                auto np = pathes[iter & 0x1][ni];
                auto pz = iv2{np.pixel & half_mask, np.pixel >> half_bits};
                auto pw = iv2{p.pixel & half_mask, p.pixel >> half_bits};

                sp.ctx.pixel = py; sp.start();
                auto gixel = ctx.film(ctx.filter, py, sp.generate_pixel_2d());
                MTT_OPT_OR_CALLBACK(ns, photo::Camera{}.sample(
                    ctx.lens, gixel.position, gixel.dxdy, sp.generate_2d()
                ), stl::abort("ray generation failed"););
                ns.ray_differential = ct ^ ns.ray_differential;
                ns.default_differential = ct ^ ns.default_differential;

                sp.ctx.pixel = pz;
                sp.ctx.dim = 5; // skip lambda generation
                sp.start();
                auto r = Ray{
                    ctx.accel, ctx.emitter,
                    sp, np.Li.lambda,
                    s.ray_differential,
                    s.default_differential,
                    ct, px, image.size,
                    ctx.sample_index, depth,
                };
                MTT_OPT_OR_CONTINUE(c, replay(r, np, u[1]));

                sp.ctx.pixel = pw;
                sp.ctx.dim = 5;
                auto nr = Ray{
                    ctx.accel, ctx.emitter,
                    sp, p.Li.lambda,
                    ns.ray_differential,
                    ns.default_differential,
                    ct, py, image.size,
                    ctx.sample_index, depth,
                };
                MTT_OPT_OR_CONTINUE(d, replay(nr, p, u[2]));

                if (false
                || !math::isfinite(c.J) || !math::isfinite(d.J)
                || !math::isfinite(d.r.p_hat)
                || !math::isfinite(c.Li.value)) continue;

                valid++;
                auto Mi = np.r.M;
                auto Mc = p.r.M;
                auto pni_hat = math::guarded_div(np.r.p_hat, c.J);
                auto pnc_hat = c.r.p_hat;
                auto mi = math::guarded_div(Mi * pni_hat, Mi * pni_hat + Mc / N * pnc_hat);
                auto pci_hat = d.r.p_hat * d.J;
                auto pcc_hat = p.r.p_hat;
                canonical += math::guarded_div(Mc / N * pcc_hat, Mi * pci_hat + Mc / N * pcc_hat);

                c.r.shift(c.J, mi);
                dst.merge(c, u[3]);
            }

            sp.ctx.pixel = px;
            sp.ctx.dim = (spatial_samples * 4 + 1) * iter + spatial_samples * 4;
            sp.start();
            p.r.shift(1.f, canonical);
            dst.merge(p, sp.generate_1d());
            dst.r.finalize();
            dst.r.W = math::guarded_div(dst.r.W, f32(valid + 1));

            if (iter < reuse_iterations - 1) pathes[(iter + 1) & 0x1][pi] = dst;
            else {
                dst.Li.value *= dst.r.W / s.pdf;
                if (!math::isfinite(dst.Li.value))
                    stl::abort("invalid reuse appears in pixel {} sample {}", px, ctx.sample_index);
                fixel = dst.Li;
            }
        };

        stl::scheduler::sync_parallel(uzv2{size}, trace);
        for (auto i = 0; i < reuse_iterations; i++, iter++)
            stl::scheduler::sync_parallel(uzv2{size}, reuse);
    }

    auto Restir_Integrator::wave(ref<Context> ctx) const noexcept -> void {
        auto threads = uv3{ctx.image->width, ctx.image->height, 1};
        auto group = uv3{8, 8, 1};

        constants->sample_index = ctx.sample_index;
        auto args = encoder::Argument_Encoder{ctx.render, arguments.get()};
        args.push(*constants, {offsetof(Constants, sample_index), sizeof(Constants::sample_index)});
        args.submit();

        auto liberate = encoder::Transfer_Encoder{ctx.render};
        liberate.liberate(*ctx.image);
        liberate.submit();

        auto dispatch = [&](mut<shader::Pipeline> ppl, u32 iter) {
            constants->iter = iter;
            auto args = encoder::Argument_Encoder{ctx.render, arguments.get()};
            args.push(*constants, {offsetof(Constants, iter), sizeof(Constants::iter)});
            args.submit();

            auto pipeline = encoder::Pipeline_Encoder{ctx.render, ppl};
            pipeline.bind();
            pipeline.dispatch(threads, group);
            pipeline.submit();
        };

        auto barrier = [&]() {
            auto liberate = encoder::Transfer_Encoder{ctx.render};
            for (auto& path: pathes)
                liberate.liberate(*mut<opaque::Buffer>(path.handle));
            liberate.submit();
        };

        barrier();
        dispatch(integrate.get(), 0);
        for (auto i = 0; i < reuse_iterations; i++) {
            barrier();
            dispatch(restir.get(), i);
        }
    }

    auto Restir_Integrator::sample(ref<Ray> r) const noexcept -> opt<Path> {
        auto path = Path{
            .Li = {r.lambda, fv4{0.f}},
            .pixel = (r.pixel[0] & half_mask) | ((r.pixel[1] << 16)),
        };
        auto beta = fv4{1.f};

        auto depth = 0u;
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
            auto direct_ctx = trace_ctx;
            auto u = fv4{r.sampler.generate_2d(), r.sampler.generate_2d()};
            if (specular) return;
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
                path.estimate({trace_ctx.lambda, mis_u * gamma * l_intr.L}, rr, u[3]);
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
            path.estimate({trace_ctx.lambda, mis_u * gamma * l_intr.L}, rr, u[3]);
        };

        while (true) {
            depth++;
            if (depth >= r.max_depth) break;

            auto q = math::max(beta * rr);
            if (q < 1.f) {
                auto rr_u = r.sampler.generate_1d();
                if (rr_u > q) break;
                else rr /= q;
            }

            if (depth > 1) direct_lighting();
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
                path.estimate({trace_ctx.lambda, beta * mis_w * l_intr.L}, rr, eu);
                break;
            }

            auto& acc = *acc_opt;
            auto& intr = *acc.intr_opt;
            auto div = acc.divider;
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
                path.estimate({trace_ctx.lambda, beta * mis_w * mat_intr.emission}, rr, eu);
            }

            auto tbn = math::transpose(fm33{intr.tn, intr.bn, intr.n});
            intr.n = tbn | mat_intr.normal;

            bsdf = std::move(mat_intr.bsdf);
            auto bt = math::Transform{};
            bt.transform = fm44{fq::from_rotation_between(intr.n, {0.f, 1.f, 0.f})};
            bt.inv_transform = math::transpose(bt.transform);
            auto cu = r.sampler.generate_1d();
            auto du = r.sampler.generate_2d();

            auto b_ctx = bt | trace_ctx;
            b_ctx.r.d = math::normalize(b_ctx.r.d);
            MTT_OPT_OR_BREAK(b_intr, bsdf.sample(b_ctx, {cu, du[0], du[1]}));

            specular = bsdf.flags() & bsdf::Flags::specular;
            auto reflected = b_intr.wi[1] >= 0.f;
            auto trace_n = (b_intr.wi[1] < 0.f ? -1.f : 1.f) * intr.n;
            auto trace_p = intr.p + 0.001f * trace_n;
            b_intr.f *= math::abs(math::unit_to_cos_theta(b_intr.wi));
            b_intr.wi = math::normalize(bt ^ math::expand(b_intr.wi, 0.f));

            beta *= b_intr.f / b_intr.pdf;
            history_ctx.r = trace_ctx.r;
            history_ctx.n = trace_ctx.n;
            trace_ctx.r = {trace_p, b_intr.wi};
            trace_ctx.n = trace_n;
            f = b_intr.f;
            p = b_intr.pdf;

            if (mat_intr.degraded && !reflected && !math::constant(trace_ctx.lambda)) {
                beta = fv4{beta[0]}; f = fv4{f[0]};
                trace_ctx.lambda = fv4{trace_ctx.lambda[0]};
                history_ctx.lambda = trace_ctx.lambda;
            }

            if (connectable && b_intr.connectable && intr.t > reconnection_t) {
                path.beta = beta;
                path.p = intr.p;
                path.t = intr.t;
                path.wi = b_intr.wi;
                path.cos_theta = math::abs(math::unit_to_cos_theta(b_ctx.r.d));
                path.pdf = {p, b_intr.pdf};
                path.depth = depth - 1;
            }
            connectable = (path.depth == 0) && b_intr.connectable;
        }

        if (!math::isfinite(path.Li.value)) return {};
        // support set of different path depth is not intersected
        path.r.M = 1.f;
        path.r.finalize();
        return path;
    }

    auto Restir_Integrator::replay(ref<Ray> r, cref<Path> np, f32 u) const noexcept -> opt<Path> {
        auto p = np; p.J = 1.f;
        auto depth = 0u;
        auto beta = fv4{1.f};
        auto connected = false;

        auto trace_ctx = math::Context{};
        auto history_ctx = math::Context{};
        trace_ctx.r = r.ray_differential.r;
        trace_ctx.lambda = r.lambda;

        auto acc_opt = opt<accel::Interaction>{};
        auto& rdiff = r.ray_differential;
        auto& ddiff = r.default_differential;
        auto& ct = r.render_to_camera;

        while (true) {
            depth++;
            if (depth > (p.depth + connected)) break;
            // skip direct lighting
            if (depth > 1) { r.sampler.generate_2d(); r.sampler.generate_2d(); }
            // skip final lighting
            r.sampler.generate_1d();

            acc_opt = r.accel(trace_ctx.r, trace_ctx.n);
            if (!acc_opt) break;
            auto& acc = *acc_opt;
            auto& intr = *acc.intr_opt;
            if (connected) {
                auto rt = math::length(p.p - trace_ctx.r.o);
                if (rt - intr.t > 0.001f * rt) break;
            }
            auto div = acc.divider;
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
            auto tbn = math::transpose(fm33{intr.tn, intr.bn, intr.n});
            intr.n = tbn | mat_intr.normal;

            if (mat_intr.degraded && !math::constant(trace_ctx.lambda)) {
                beta = fv4{beta[0]};
                trace_ctx.lambda = fv4{trace_ctx.lambda[0]};
                history_ctx.lambda = trace_ctx.lambda;
            }

            auto bsdf = std::move(mat_intr.bsdf);
            auto bt = math::Transform{};
            bt.transform = fm44{fq::from_rotation_between(intr.n, {0.f, 1.f, 0.f})};
            bt.inv_transform = math::transpose(bt.transform);
            auto b_ctx = bt | trace_ctx;
            b_ctx.r.d = math::normalize(b_ctx.r.d);
            auto cu = r.sampler.generate_1d();
            auto du = r.sampler.generate_2d();

            auto b_intr = bsdf::Interaction{};
            if (depth == p.depth) {
                auto wi = math::normalize(p.p - intr.p);
                wi = fv3{bt | fv4{wi, 0.f}};
                MTT_OPT_OR_BREAK(c_intr, bsdf(b_ctx.r.d, wi, cu));
                b_intr = c_intr;
                p.J *= b_intr.pdf / math::prod(p.pdf) * math::sqr(p.t) / p.cos_theta;
                p.pdf[0] = b_intr.pdf;
                connected = true;
            } else if (depth == p.depth + 1) {
                auto wi = fv3{bt | fv4{p.wi, 0.f}};
                MTT_OPT_OR_BREAK(c_intr, bsdf(b_ctx.r.d, wi, cu));
                b_intr = c_intr;
                b_intr.f *= math::abs(math::unit_to_cos_theta(b_intr.wi));
                beta *= b_intr.f / b_intr.pdf;
                p.Li.value = p.Li.value / p.beta * beta;
                p.beta = beta;
                p.t = intr.t;
                p.pdf[1] = b_intr.pdf;
                p.cos_theta = math::abs(math::unit_to_cos_theta(b_ctx.r.d));
                p.J *= b_intr.pdf / math::sqr(p.t) * p.cos_theta;
                p.r.p_hat = math::max(p.Li.value);
                return p;
            } else {
                MTT_OPT_OR_BREAK(c_intr, bsdf.sample(b_ctx, {cu, du[0], du[1]}));
                b_intr = c_intr;
            }

            auto trace_n = (b_intr.wi[1] < 0.f ? -1.f : 1.f) * intr.n;
            auto trace_p = intr.p + 0.001f * trace_n;
            b_intr.f *= math::abs(math::unit_to_cos_theta(b_intr.wi));
            b_intr.wi = math::normalize(bt ^ math::expand(b_intr.wi, 0.f));

            beta *= b_intr.f / b_intr.pdf;
            history_ctx.r = trace_ctx.r;
            history_ctx.n = trace_ctx.n;
            trace_ctx.r = {trace_p, b_intr.wi};
            trace_ctx.n = trace_n;
        }

        return {};
    }
}
