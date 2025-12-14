#pragma once
#include <metatron/core/math/matrix.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/ranges.hpp>
#include <metatron/core/stl/stack.hpp>

namespace mtt::math {
    // initially implements Piecewise_Distribution<usize n>,
    // but embedded buf<Element> prohibits parallel release,
    // harms cache locality and produces a large number of discrete bufs.
    // sequential == piecewise<1>, planar == piecewise<2>.

    struct Sequential_Distribution final {
        Sequential_Distribution() noexcept = default;

        Sequential_Distribution(
            std::span<f32> data,
            i32 dimensions,
            f32 low,
            f32 high
        ) noexcept: low(low), high(high), dim(dimensions) {
            integral = f32{0};
            delta = (high - low) / f32(dim);
            rows = dim;
            cdf = dim + 1;
            cdf[0] = 0.f;

            for (auto i = 0; i < dim; ++i) {
                rows[i] = abs(data[i]);
                cdf[i + 1] = rows[i] * delta + cdf[i];
            }
            integral = cdf[dim];

            for (auto i = 1; i <= dim; ++i)
                cdf[i] = integral == 0.f ? f32(i) / f32(dim) : cdf[i] / integral;
        }

        auto sample(f32 u) const noexcept -> f32 {
            auto idx = 1;
            for (; idx < dim && cdf[idx] <= u; ++idx);
            idx--;
            auto du = guarded_div(u - cdf[idx], cdf[idx + 1] - cdf[idx]);
            return lerp(low, high, (f32(idx) + du) / f32(dim));
        }

        auto pdf(f32 p) const noexcept -> f32 {
            auto idx = math::clamp(
                i32((p - low) / delta),
                0, dim - 1
            );
            return (cdf[idx + 1] - cdf[idx]) / delta;
        }

    private:
        buf<f32> rows;
        buf<f32> cdf;

        i32 dim;
        f32 low{};
        f32 high{};
        f32 delta{};
        f32 integral{};

        template<usize m>
        friend struct Piecewise_Distribution;
    };

    struct Planar_Distribution final {
        Planar_Distribution() noexcept = default;

        Planar_Distribution(
            std::span<f32> data,
            cref<Vector<i32, 2>> dimensions,
            cref<Vector<f32, 2>> low,
            cref<Vector<f32, 2>> high
        ) noexcept: low(low), high(high), dim(dimensions) {
            integral = 0;
            delta = (high - low) / fv2(dim);
            rows = dim[0] * dim[1];
            cols = dim[0];
            row_cdf = dim[0] * (dim[1] + 1);
            col_cdf = dim[0] + 1;
            col_cdf[0] = 0.f;

            for (auto i = 0; i < dim[0]; ++i) {
                auto data_offset = i * dim[1];
                auto cdf_offset = i * (dim[1] + 1);
                row_cdf[cdf_offset] = 0;
                for (auto j = 0; j < dim[1]; ++j) {
                    auto k = data_offset + j;
                    auto l = cdf_offset + j + 1;
                    rows[k] = abs(data[k]);
                    row_cdf[l] = rows[k] * delta[1] + row_cdf[l - 1];
                }
                cols[i] = row_cdf[cdf_offset + dim[1]];
                col_cdf[i + 1] = cols[i] * delta[0] + col_cdf[i];
                for (auto j = 1; j <= dim[1]; ++j) {
                    auto k = cdf_offset + j;
                    row_cdf[k] = cols[i] == 0.f ? f32(j) / f32(dim[1]) : row_cdf[k] / cols[i];
                }
            }

            integral = col_cdf[dim[0]];
            for (auto i = 1; i <= dim[0]; ++i)
                col_cdf[i] = integral == 0.f ? f32(i) / f32(dim[0]) : col_cdf[i] / integral;
        }

        auto sample(cref<Vector<f32, 2>> u) const noexcept -> Vector<f32, 2> {
            auto ci = 1;
            for (; ci < dim[0] && col_cdf[ci] <= u[0]; ++ci);
            ci--;
            auto cu = guarded_div(u[0] - col_cdf[ci], col_cdf[ci + 1] - col_cdf[ci]);
            auto cp = lerp(low[0], high[0], (f32(ci) + cu) / f32(dim[0]));

            auto offset = ci * (dim[1] + 1);
            auto ri = 1;
            for (; ri < dim[1] && row_cdf[offset + ri] <= u[1]; ++ri);
            ri--; offset += ri;
            auto ru = guarded_div(u[1] - row_cdf[offset], row_cdf[offset + 1] - row_cdf[offset]);
            auto rp = lerp(low[1], high[1], (f32(ri) + ru) / f32(dim[1]));

            return {cp, rp};
        }

        auto pdf(cref<Vector<f32, 2>> p) const noexcept -> f32 {
            auto ci = math::clamp(
                i32((p[0] - low[0]) / delta[0]),
                0, dim[0] - 1
            );
            auto cp = (col_cdf[ci + 1] - col_cdf[ci]) / delta[0];

            auto offset = ci * (dim[1] + 1);
            auto ri = math::clamp(
                i32((p[1] - low[1]) / delta[1]),
                0, dim[1] - 1
            );
            offset += ri;
            auto rp = (row_cdf[offset + 1] - row_cdf[offset]) / delta[1];

            return cp * rp;
        }

    private:
        buf<f32> rows;
        buf<f32> row_cdf;
        buf<f32> cols;
        buf<f32> col_cdf;
        f32 integral;

        Vector<i32, 2> dim;
        Vector<f32, 2> low{};
        Vector<f32, 2> high{};
        Vector<f32, 2> delta{};
    };
}
