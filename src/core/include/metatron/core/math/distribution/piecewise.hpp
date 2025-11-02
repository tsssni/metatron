#pragma once
#include <metatron/core/math/matrix.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/ranges.hpp>
#include <vector>

namespace mtt::math {
    template<usize n>
    struct Piecewise_Distribution final {
        using Element = std::conditional_t<n == 1, f32, Piecewise_Distribution<n - 1>>;

        Piecewise_Distribution() noexcept = default;

        Piecewise_Distribution(
            std::span<f32> data,
            Vector<usize, n> const& dimensions,
            Vector<f32, n> const& low,
            Vector<f32, n> const& high
        ) noexcept {
            integral = f32{0};
            this->low = low[0];
            this->high = high[0];

            dim = dimensions[0];
            delta = (this->high - this->low) / dim;
            rows.resize(dim);
            cdf.resize(dim + 1);
            auto size = prod(dimensions) / dim;

            for (auto i = 0uz; i < dim; ++i) {
                if constexpr (n == 1uz) {
                    rows[i] = abs(data[i]);
                    cdf[i + 1] = rows[i];
                } else {
                    rows[i] = Element{std::span{&data[i * size], size}, cut(dimensions), cut(low), cut(high)};
                    cdf[i + 1] = rows[i].integral;
                }
                cdf[i + 1] = cdf[i + 1] * delta + cdf[i];
            }

            integral = cdf.back();
            for (auto i = 1uz; i <= dim; ++i)
                if (integral == 0.f) cdf[i] = f32(i) / f32(dim);
                else cdf[i] /= integral;
        }

        auto sample(Vector<f32, n> const& u) const noexcept -> Vector<f32, n> {
            auto idx = 1uz;
            for (; idx < dim && cdf[idx] <= u[0]; ++idx);
            idx--;

            auto du = guarded_div(u[0] - cdf[idx], cdf[idx + 1uz] - cdf[idx]);
            auto p = lerp(low, high, (f32(idx) + du) / f32(dim));

            if constexpr (n == 1uz) return {p};
            else return consume(rows[idx].sample(cut(u)), p);
        }

        auto pdf(Vector<f32, n> const& p) const noexcept -> f32 {
            auto idx = math::clamp(
                usize((p[0] - low) / delta),
                0uz, dim - 1uz
            );
            auto prob = (cdf[idx + 1] - cdf[idx]) / delta;
            if constexpr (n == 1uz) return prob;
            else return rows[idx].pdf(cut(p)) * prob;
        }

    private:
        std::vector<Element> rows{};
        std::vector<f32> cdf{};

        usize dim;
        f32 low{};
        f32 high{};
        f32 delta{};
        f32 integral{};

        template<usize m>
        friend struct Piecewise_Distribution;
    };
}
