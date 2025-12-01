#include <metatron/resource/shape/mesh.hpp>
#include <metatron/core/math/transform.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/math/distribution/linear.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <metatron/core/stl/print.hpp>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace mtt::shape {
    Mesh::Mesh(cref<Descriptor> desc) noexcept {
        auto path = stl::filesystem::find(desc.path);
        auto importer = Assimp::Importer{};
        auto* scene = importer.ReadFile(path.c_str(), 0
            | aiProcess_FlipUVs
            | aiProcess_FlipWindingOrder
            | aiProcess_MakeLeftHanded
        );

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->HasMeshes()) {
            std::println("assimp error: while loading {}: {}", desc.path, importer.GetErrorString());
            std::abort();
        }
        auto* mesh = scene->mMeshes[0];

        indices = mesh->mNumFaces;
        for (auto i = 0uz; i < mesh->mNumFaces; ++i) {
            auto face = mesh->mFaces[i];
            indices[i] = {
                face.mIndices[0],
                face.mIndices[1],
                face.mIndices[2]
            };
        }

        vertices = mesh->mNumVertices;
        normals = mesh->mNumVertices;
        uvs = mesh->mNumVertices;
        for (auto i = 0uz; i < mesh->mNumVertices; ++i) {
            vertices[i] = {
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z
            };
            normals[i] = {
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            };
            uvs[i] = mesh->mTextureCoords[0]
            ? fv2{
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            }
            : 1.f
            * math::cartesian_to_unit_spherical(math::normalize(vertices[i]))
            / fv2{math::pi, 2.f * math::pi};
        }

        dpdu = indices.size();
        dpdv = indices.size();
        dndu = indices.size();
        dndv = indices.size();

        for (auto i = 0uz; i < indices.size(); ++i) {
            auto prim = indices[i];
            auto v = fm33{
                vertices[prim[0]],
                vertices[prim[1]],
                vertices[prim[2]],
            };
            auto n = fm33{
                normals[prim[0]],
                normals[prim[1]],
                normals[prim[2]],
            };
            auto uv = fm32{
                uvs[prim[0]],
                uvs[prim[1]],
                uvs[prim[2]],
            };

            auto A = fm22{uv[0] - uv[2], uv[1] - uv[2]};
            auto dpduv_opt = math::cramer(A,
                fm23{v[0] - v[2], v[1] - v[2]}
            );
            // remove parallel dpduv
            if (dpduv_opt) {
                auto dpduv = dpduv_opt.value();
                auto perp = math::cross(dpduv[0], dpduv[1]);
                if (math::length(perp) == 0.f) dpduv_opt.reset();
            }
            // fallback to make sure normal is correct
            if (!dpduv_opt) {
                auto n = math::normalize(math::cross(v[2] - v[0], v[1] - v[0]));
                dpduv_opt = math::orthogonalize(n);
            }

            auto dnduv_opt = math::cramer(A,
                fm23{n[0] - n[2], n[1] - n[2]}
            );
            if (!dnduv_opt) {
                auto dn = math::normalize(math::cross(n[2] - n[0], n[1] - n[0]));
                dnduv_opt = math::length(dn) == 0
                ? fm23{0.f}
                : math::orthogonalize(dn);
            }

            auto dpduv = dpduv_opt.value();
            auto dnduv = dnduv_opt.value();
            dpdu[i] = dpduv[0];
            dpdv[i] = dpduv[1];
            dndu[i] = dnduv[0];
            dndv[i] = dnduv[1];
        }
    }

    auto Mesh::size() const noexcept -> usize {
        return indices.size();
    }

    auto Mesh::bounding_box(
        cref<fm44> t, usize idx
    ) const noexcept -> math::Bounding_Box {
        auto prim = indices[idx];
        auto v = math::Vector<fv4, 3>{
            t | math::expand(vertices[prim[0]], 1.f),
            t | math::expand(vertices[prim[1]], 1.f),
            t | math::expand(vertices[prim[2]], 1.f)
        };
        auto p_min = math::min(v[0], v[1], v[2]);
        auto p_max = math::max(v[0], v[1], v[2]);
        return {p_min, p_max};
    }

    auto Mesh::operator()(
        cref<math::Ray> r, cref<fv3> np, usize idx
    ) const noexcept -> opt<Interaction> {
        MTT_OPT_OR_RETURN(isec, intersect(r, idx), {});
        auto bary = math::shrink(isec);
        auto t = isec[3];
        auto pdf = this->pdf(r, np, idx);
        auto p = blerp(vertices, bary, idx);
        auto n = blerp(normals, bary, idx);
        auto tn = math::gram_schmidt(dpdu[idx], n);
        auto bn = math::cross(tn, n);
        auto uv = blerp(uvs, bary, idx);

        return shape::Interaction{
            p, n, tn, bn, uv, t, pdf,
            dpdu[idx], dpdv[idx],
            dndu[idx], dndv[idx],
        };
    }

    auto Mesh::sample(
        cref<math::Context> ctx, cref<fv2> u, usize idx
    ) const noexcept -> opt<Interaction> {
        auto prim = indices[idx];
        auto validate_vector = [](cref<fv3> v) -> bool {
            return math::dot(v, v) >= math::epsilon<f32>;
        };

        auto a = math::normalize(vertices[prim[0]] - ctx.r.o);
        auto b = math::normalize(vertices[prim[1]] - ctx.r.o);
        auto c = math::normalize(vertices[prim[2]] - ctx.r.o);
        if (false
        || validate_vector(a)
        || validate_vector(b)
        || validate_vector(c)) return {};

        auto n_ab = math::normalize(math::cross(b, a));
        auto n_bc = math::normalize(math::cross(c, b));
        auto n_ca = math::normalize(math::cross(a, c));
        if (false
        || validate_vector(n_ab)
        || validate_vector(n_bc)
        || validate_vector(n_ca)) return {};

        auto alpha = math::angle(n_ab, -n_ca);
        auto beta = math::angle(n_bc, -n_ab);
        auto gamma = math::angle(n_ca, -n_bc);

        auto A_pi = alpha + beta + gamma;
        auto A_1_pi = math::lerp(math::pi, A_pi, u[0]);

        auto phi = A_1_pi - alpha;
        auto cos_a = std::cos(alpha);
        auto sin_a = std::sin(alpha);
        auto cos_p = std::cos(phi);
        auto sin_p = std::sin(phi);

        auto k_1 = cos_p + cos_a;
        auto k_2 = sin_p - sin_a * dot(a, b);
        auto cos_ac1 = math::guarded_div(
            k_2 + (k_2 * cos_p - k_1 * sin_p) * cos_a,
            (k_2 * sin_p + k_1 * cos_p) * sin_a
        );
        auto sin_ac1 = math::sqrt(1.f - cos_ac1 * cos_ac1);
        auto c_1 = cos_ac1 * a + sin_ac1 * math::normalize(math::gram_schmidt(c, a));

        auto cos_bc1 = math::dot(b, c_1);
        auto cos_bc2 = 1.f - u[1] * (1.f - cos_bc1);
        auto sin_bc2 = math::sqrt(1.f - cos_bc2 * cos_bc2);
        auto d = cos_bc2 * b + sin_bc2 * math::normalize(math::gram_schmidt(c_1, b));

        auto v = math::Vector<fv3, 3>{
            vertices[prim[0]],
            vertices[prim[1]],
            vertices[prim[2]],
        };
        MTT_OPT_OR_RETURN(cramer, math::cramer(
            math::transpose(fm33{-d, v[1] - v[0], v[2] - v[0]}),
            ctx.r.o - v[0]
        ), {});
        auto [t, b_1, b_2] = cramer;
        b_1 = math::clamp(b_1, 0.f, 1.f);
        b_2 = math::clamp(b_2, 0.f, 1.f);
        if (b_1 + b_2 > 1.f) {
            b_1 /= (b_1 + b_2);
            b_2 /= (b_1 + b_2);
        }
        auto bary = fv3{1.f - b_1 - b_2, b_1, b_2};

        auto pdf = this->pdf({ctx.r.o, d}, ctx.n, idx);
        auto p = ctx.r.o + t * d;
        auto n = blerp(normals, bary, idx);
        auto tn = math::gram_schmidt(dpdu[idx], n);
        auto bn = math::cross(tn, n);
        auto uv = blerp(uvs, bary, idx);

        return shape::Interaction{
            p, n, tn, bn, uv, t, pdf,
            dpdu[idx], dpdv[idx],
            dndu[idx], dndv[idx],
        };
    }

    auto Mesh::query(
        cref<math::Ray> r, usize idx
    ) const noexcept -> opt<f32> {
        MTT_OPT_OR_RETURN(isec, intersect(r, idx), {});
        return isec[3];
    }

    auto Mesh::intersect(
        cref<math::Ray> r, usize idx
    ) const noexcept -> opt<fv4> {
        auto rs = r.d;
        auto ri = math::maxi(math::abs(rs));
        std::swap(rs[2], rs[ri]);
        auto rt = fv3{
            -rs[0], -rs[1], 1.f
        } / rs[2];

        auto local_to_shear = [&](auto const& x) {
            auto y = x - r.o;
            std::swap(y[2], y[ri]);
            auto z = y[2] * rt;
            y[2] = 0.f;
            return y + z;
        };

        auto prim = indices[idx];
        auto v = math::Vector<fv4, 3>{
            local_to_shear(vertices[prim[0]]),
            local_to_shear(vertices[prim[1]]),
            local_to_shear(vertices[prim[2]]),
        };

        auto ef = [](
            fv2 p,
            fv2 v0,
            fv2 v1
        ) -> f32 {
            return math::determinant(fm22{v1 - v0, p - v0});
        };
        auto e = fv3{
            ef({0.f}, v[1], v[2]),
            ef({0.f}, v[2], v[0]),
            ef({0.f}, v[0], v[1]),
        };
        auto det = math::sum(e);

        if (false
        || math::abs(det) < math::epsilon<f32>
        || std::signbit(e[0]) != std::signbit(e[1])
        || std::signbit(e[1]) != std::signbit(e[2])
        ) return {};

        auto bary = e / det;
        auto t = math::blerp(v, bary)[2];
        if (t < math::epsilon<f32>) return {};
        return fv4{bary, t};
    }

    auto Mesh::pdf(
        cref<math::Ray> r, cref<fv3> np, usize idx
    ) const noexcept -> f32 {
        auto prim = vertices[idx];
        auto a = math::normalize(vertices[prim[0]] - r.o);
        auto b = math::normalize(vertices[prim[1]] - r.o);
        auto c = math::normalize(vertices[prim[2]] - r.o);

        auto n_ab = math::normalize(math::cross(b, a));
        auto n_bc = math::normalize(math::cross(c, b));
        auto n_ca = math::normalize(math::cross(a, c));

        auto alpha = math::angle(n_ab, -n_ca);
        auto beta = math::angle(n_bc, -n_ab);
        auto gamma = math::angle(n_ca, -n_bc);

        auto c_2 = math::normalize(r.d);
        auto c_1 = math::normalize(math::cross(math::cross(b, c_2), math::cross(c, a)));
        if (math::dot(c_1, a + c) < 0.f) c_1 *= -1.f;

        auto u = fv2{};
        auto pdf = 1.f;
        u[1] = math::guarded_div(1.f - math::dot(b, c_2), (1.f - math::dot(b, c_1)));
        pdf = math::guarded_div(pdf, 1.f - math::dot(b, c_1));
        if (math::dot(a, c_1) > 0.99999847691f) { // 0.1 degrees
            u[0] = 0.f;
        } else {
            auto n_bc1 = math::normalize(math::cross(c_1, b));
            auto n_c1a = math::normalize(math::cross(a, c_1));
            auto A = alpha + beta + gamma - math::pi;
            pdf = math::guarded_div(pdf, A);

            if (math::length(n_bc1) < math::epsilon<f32> || math::length(n_c1a) < math::epsilon<f32>) {
                u = {0.5f};
            } else {
                auto A_1 = alpha + math::angle(n_bc1, -n_ab) + math::angle(n_c1a, -n_bc1) - math::pi;
                u[1] = math::guarded_div(A_1, A);
            }
        }

        if (np != fv3{0.f}) {
            auto distr = math::Bilinear_Distribution{
                math::dot(np, b),
                math::dot(np, a),
                math::dot(np, b),
                math::dot(np, c),
            };
            pdf *= distr.pdf(u);
        }
        return pdf;
    }
}
