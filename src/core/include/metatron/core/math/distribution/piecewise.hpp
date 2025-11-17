#pragma once
#include <metatron/core/math/matrix.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/ranges.hpp>
#include <metatron/core/stl/stack.hpp>
#include <vector>

namespace mtt::math {
    template<usize n>
    struct Piecewise_Distribution final {
        using Element = std::conditional_t<n == 1, f32, Piecewise_Distribution<n - 1>>;

        Piecewise_Distribution() noexcept = default;

        Piecewise_Distribution(
            std::span<f32> data,
            cref<Vector<i32, n>> dimensions,
            cref<Vector<f32, n>> low,
            cref<Vector<f32, n>> high
        ) noexcept {
            integral = f32{0};
            this->low = low[0];
            this->high = high[0];

            dim = dimensions[0];
            delta = (this->high - this->low) / dim;
            auto rows = std::vector<Element>(dim);
            auto cdf = std::vector<f32>(dim + 1, 0.f);
            auto size = prod(dimensions) / dim;

            for (auto i = 0; i < dim; ++i) {
                if constexpr (n == 1) {
                    rows[i] = abs(data[i]);
                    cdf[i + 1] = rows[i];
                } else {
                    rows[i] = Element{
                        std::span{&data[i * size], usize(size)},
                        cut(dimensions), cut(low), cut(high)
                    };
                    cdf[i + 1] = rows[i].integral;
                }
                cdf[i + 1] = cdf[i + 1] * delta + cdf[i];
            }

            integral = cdf.back();
            for (auto i = 1; i <= dim; ++i)
                cdf[i] = integral == 0.f ? f32(i) / f32(dim) : cdf[i] /= integral;

            auto lock = stl::stack::instance().lock();
            this->rows = buf<Element>{rows};
            this->cdf = buf<f32>{cdf};
        }

        auto sample(cref<Vector<f32, n>> u) const noexcept -> Vector<f32, n> {
            auto idx = 1;
            for (; idx < dim && cdf[idx] <= u[0]; ++idx);
            idx--;

            auto du = guarded_div(u[0] - cdf[idx], cdf[idx + 1] - cdf[idx]);
            auto p = lerp(low, high, (f32(idx) + du) / f32(dim));

            if constexpr (n == 1) return {p};
            else return consume(rows[idx].sample(cut(u)), p);
        }

        auto pdf(cref<Vector<f32, n>> p) const noexcept -> f32 {
            auto idx = math::clamp(
                i32((p[0] - low) / delta),
                0, dim - 1
            );
            auto prob = (cdf[idx + 1] - cdf[idx]) / delta;
            if constexpr (n == 1) return prob;
            else return rows[idx].pdf(cut(p)) * prob;
        }

    private:
        buf<Element> rows;
        buf<f32> cdf;

        i32 dim;
        f32 low{};
        f32 high{};
        f32 delta{};
        f32 integral{};

        template<usize m>
        friend struct Piecewise_Distribution;
    };
}
