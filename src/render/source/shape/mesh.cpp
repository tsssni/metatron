#include <metatron/render/shape/mesh.hpp>
#include <metatron/core/distribution/linear.hpp>
#include <metatron/core/math/cmath.hpp>
#include <metatron/core/math/sphere.hpp>
#include <metatron/core/stl/filesystem.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace mtt::render {

Mesh::Mesh(std::string_view path) noexcept {
    auto file = core::filesystem::find(path);
    auto importer = Assimp::Importer{};
    auto* scene = importer.ReadFile(file.c_str(), 0 | aiProcess_FlipUVs | aiProcess_FlipWindingOrder | aiProcess_MakeLeftHanded);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->HasMeshes())
        core::abort("assimp error: while loading {}: {}", path, importer.GetErrorString());
    auto* mesh = scene->mMeshes[0];

    auto indices = std::vector<uv3>(mesh->mNumFaces);
    for (auto i = 0uz; i < mesh->mNumFaces; ++i) {
        auto face = mesh->mFaces[i];
        indices[i] = {
            face.mIndices[0],
            face.mIndices[1],
            face.mIndices[2]
        };
    }

    auto vertices = std::vector<fv3>(mesh->mNumVertices);
    auto normals = std::vector<fv3>(mesh->mNumVertices);
    auto uvs = std::vector<fv2>(mesh->mNumVertices);
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
            ? fv2{mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y}
            : 1.f * core::cartesian_to_unit_spherical(core::normalize(vertices[i])) / fv2{core::pi, 2.f * core::pi};
    }

    auto dpdu = std::vector<fv3>(indices.size());
    auto dpdv = std::vector<fv3>(indices.size());
    auto dndu = std::vector<fv3>(indices.size());
    auto dndv = std::vector<fv3>(indices.size());

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
        auto dpduv = core::cramer(A, fm23{v[0] - v[2], v[1] - v[2]});
        // remove parallel dpduv
        // fallback to make sure normal is correct
        if (false
            || !core::isfinite(dpduv[0])
            || !core::isfinite(dpduv[1])
            || core::length(core::cross(dpduv[0], dpduv[1])) < core::epsilon<f32>) {
            dpduv = core::orthogonalize(normals[prim[0]]);
            if (dpduv[0] == fv3{0}) core::print("idx: {} {}", i, n);
        }

        auto dnduv = core::cramer(A, fm23{n[0] - n[2], n[1] - n[2]});
        if (!core::isfinite(dnduv[0]) || !core::isfinite(dnduv[1])) {
            auto dn = core::normalize(core::cross(n[2] - n[0], n[1] - n[0]));
            dnduv = core::length(dn) == 0 ? fm23{0.f} : core::orthogonalize(dn);
        }

        dpdu[i] = dpduv[0];
        dpdv[i] = dpduv[1];
        dndu[i] = dnduv[0];
        dndv[i] = dnduv[1];
    }
    num_prims = indices.size();
    auto store = []<typename T>(buf<T>& target, std::vector<T> const& data) {
        target = data.size();
        target.map();
        std::ranges::copy(data, target.ptr);
        target.unmap();
    };
    store(this->indices, indices);
    store(this->vertices, vertices);
    store(this->normals, normals);
    store(this->uvs, uvs);
    store(this->dpdu, dpdu);
    store(this->dpdv, dpdv);
    store(this->dndu, dndu);
    store(this->dndv, dndv);
}

auto Mesh::size() const noexcept -> usize {
    return num_prims;
}

auto Mesh::bounding_box(
    core::Transform const& t, usize idx
) const noexcept -> core::Bounding_Box {
    auto prim = indices[idx];
    auto v = core::vector<fv4, 3>{
        t | core::expand(vertices[prim[0]], 1.f),
        t | core::expand(vertices[prim[1]], 1.f),
        t | core::expand(vertices[prim[2]], 1.f)
    };
    auto p_min = core::min(v[0], v[1], v[2]);
    auto p_max = core::max(v[0], v[1], v[2]);
    return {p_min, p_max};
}

auto Mesh::operator()(
    core::Ray const& r, fv3 const& np,
    fv4 const& pos, usize idx
) const noexcept -> Shape_Interaction {
    auto bary = fv3{1.f - pos[0] - pos[1], pos[0], pos[1]};
    auto t = pos[3];
    auto pdf = this->pdf(r, np, idx);
    auto p = blerp(vertices.ptr, bary, idx);
    auto n = blerp(normals.ptr, bary, idx);
    auto tn = core::gram_schmidt(dpdu[idx], n);
    auto bn = core::cross(tn, n);
    auto uv = blerp(uvs.ptr, bary, idx);

    return {
        p,
        n,
        tn,
        bn,
        uv,
        t,
        pdf,
        dpdu[idx],
        dpdv[idx],
        dndu[idx],
        dndv[idx],
    };
}

auto Mesh::sample(
    core::Ray_Context const& ctx, fv2 const& u, usize idx
) const noexcept -> Shape_Interaction {
    auto prim = indices[idx];
    auto validate_vector = [](fv3 const& v) -> bool {
        return core::dot(v, v) >= core::epsilon<f32>;
    };

    auto a = core::normalize(vertices[prim[0]] - ctx.r.o);
    auto b = core::normalize(vertices[prim[1]] - ctx.r.o);
    auto c = core::normalize(vertices[prim[2]] - ctx.r.o);
    if (false
        || validate_vector(a)
        || validate_vector(b)
        || validate_vector(c)) return {};

    auto n_ab = core::normalize(core::cross(b, a));
    auto n_bc = core::normalize(core::cross(c, b));
    auto n_ca = core::normalize(core::cross(a, c));
    if (false
        || validate_vector(n_ab)
        || validate_vector(n_bc)
        || validate_vector(n_ca)) return {};

    auto alpha = core::angle(n_ab, -n_ca);
    auto beta = core::angle(n_bc, -n_ab);
    auto gamma = core::angle(n_ca, -n_bc);

    auto A_pi = alpha + beta + gamma;
    auto A_1_pi = core::lerp(core::pi, A_pi, u[0]);

    auto phi = A_1_pi - alpha;
    auto cos_a = core::cos(alpha);
    auto sin_a = core::sin(alpha);
    auto cos_p = core::cos(phi);
    auto sin_p = core::sin(phi);

    auto k_1 = cos_p + cos_a;
    auto k_2 = sin_p - sin_a * dot(a, b);
    auto cos_ac1 = core::guarded_div(
        k_2 + (k_2 * cos_p - k_1 * sin_p) * cos_a,
        (k_2 * sin_p + k_1 * cos_p) * sin_a
    );
    auto sin_ac1 = core::pow<1, 2>(1.f - cos_ac1 * cos_ac1);
    auto c_1 = cos_ac1 * a + sin_ac1 * core::normalize(core::gram_schmidt(c, a));

    auto cos_bc1 = core::dot(b, c_1);
    auto cos_bc2 = 1.f - u[1] * (1.f - cos_bc1);
    auto sin_bc2 = core::pow<1, 2>(1.f - cos_bc2 * cos_bc2);
    auto d = cos_bc2 * b + sin_bc2 * core::normalize(core::gram_schmidt(c_1, b));

    auto v = core::vector<fv3, 3>{
        vertices[prim[0]],
        vertices[prim[1]],
        vertices[prim[2]],
    };
    auto [t, b_1, b_2] = core::cramer(
        core::transpose(fm33{-d, v[1] - v[0], v[2] - v[0]}),
        ctx.r.o - v[0]
    );
    b_1 = core::clamp(b_1, 0.f, 1.f);
    b_2 = core::clamp(b_2, 0.f, 1.f);
    if (b_1 + b_2 > 1.f) {
        b_1 /= (b_1 + b_2);
        b_2 /= (b_1 + b_2);
    }
    auto bary = fv3{1.f - b_1 - b_2, b_1, b_2};

    auto pdf = this->pdf({ctx.r.o, d}, ctx.n, idx);
    auto p = ctx.r.o + t * d;
    auto n = blerp(normals.ptr, bary, idx);
    auto tn = core::gram_schmidt(dpdu[idx], n);
    auto bn = core::cross(tn, n);
    auto uv = blerp(uvs.ptr, bary, idx);

    return {
        p,
        n,
        tn,
        bn,
        uv,
        t,
        pdf,
        dpdu[idx],
        dpdv[idx],
        dndu[idx],
        dndv[idx],
    };
}

auto Mesh::query(
    core::Ray const& r, usize idx
) const noexcept -> fv4 {
    auto rs = r.d;
    auto ri = core::maxi(core::abs(rs));
    std::swap(rs[2], rs[ri]);
    auto rt = fv3{
                  -rs[0], -rs[1], 1.f
              }
        / rs[2];

    auto local_to_shear = [&](auto&& x) {
        auto y = x - r.o;
        std::swap(y[2], y[ri]);
        auto z = y[2] * rt;
        y[2] = 0.f;
        return y + z;
    };

    auto prim = indices[idx];
    auto v = core::vector<fv3, 3>{
        local_to_shear(vertices[prim[0]]),
        local_to_shear(vertices[prim[1]]),
        local_to_shear(vertices[prim[2]]),
    };

    auto ef = [](fv2 p, fv2 v0, fv2 v1) -> f32 {
        return core::determinant(fm22{v1 - v0, p - v0});
    };
    auto e = fv3{
        ef({0.f}, v[1], v[2]),
        ef({0.f}, v[2], v[0]),
        ef({0.f}, v[0], v[1]),
    };
    auto det = core::sum(e);

    if (false
        || core::abs(det) < core::epsilon<f32>
        || core::signbit(e[0]) != core::signbit(e[1])
        || core::signbit(e[1]) != core::signbit(e[2])) return fv4{core::inf<f32>};

    auto bary = e / det;
    auto t = core::blerp(v, bary)[2];
    if (t < core::epsilon<f32>) return fv4{core::inf<f32>};

    auto ng = core::cross(vertices[prim[1]] - vertices[prim[0]], vertices[prim[2]] - vertices[prim[0]]);
    auto face = core::dot(ng, r.d) < 0.f ? 1.f : -1.f;
    return fv4{bary[1], bary[2], face, t};
}

auto Mesh::pdf(
    core::Ray const& r, fv3 const& np, usize idx
) const noexcept -> f32 {
    auto prim = indices[idx];
    auto a = core::normalize(vertices[prim[0]] - r.o);
    auto b = core::normalize(vertices[prim[1]] - r.o);
    auto c = core::normalize(vertices[prim[2]] - r.o);

    auto n_ab = core::normalize(core::cross(b, a));
    auto n_bc = core::normalize(core::cross(c, b));
    auto n_ca = core::normalize(core::cross(a, c));

    auto alpha = core::angle(n_ab, -n_ca);
    auto beta = core::angle(n_bc, -n_ab);
    auto gamma = core::angle(n_ca, -n_bc);

    auto c_2 = core::normalize(r.d);
    auto c_1 = core::normalize(core::cross(core::cross(b, c_2), core::cross(c, a)));
    if (core::dot(c_1, a + c) < 0.f) c_1 *= -1.f;

    auto u = fv2{};
    auto pdf = 1.f;
    u[1] = core::guarded_div(1.f - core::dot(b, c_2), (1.f - core::dot(b, c_1)));
    pdf = core::guarded_div(pdf, 1.f - core::dot(b, c_1));
    if (core::dot(a, c_1) > 0.99999847691f) { // 0.1 degrees
        u[0] = 0.f;
    } else {
        auto n_bc1 = core::normalize(core::cross(c_1, b));
        auto n_c1a = core::normalize(core::cross(a, c_1));
        auto A = alpha + beta + gamma - core::pi;
        pdf = core::guarded_div(pdf, A);

        if (core::length(n_bc1) < core::epsilon<f32> || core::length(n_c1a) < core::epsilon<f32>) {
            u = {0.5f};
        } else {
            auto A_1 = alpha + core::angle(n_bc1, -n_ab) + core::angle(n_c1a, -n_bc1) - core::pi;
            u[1] = core::guarded_div(A_1, A);
        }
    }

    if (np != fv3{0.f}) {
        auto distr = core::Bilinear_Distribution{
            core::dot(np, b),
            core::dot(np, a),
            core::dot(np, b),
            core::dot(np, c),
        };
        pdf *= distr.pdf(u);
    }
    return pdf;
}

}
