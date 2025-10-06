#pragma once
#include <metatron/core/math/matrix.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/ranges.hpp>
#include <metatron/core/stl/print.hpp>
#include <vector>

namespace mtt::math {
    template<typename T, usize n>
    requires std::floating_point<T>
    struct Piecewise_Distribution final {
        using Element = std::conditional_t<n == 1, T, Piecewise_Distribution<T, n - 1>>;

        Piecewise_Distribution() noexcept = default;

        Piecewise_Distribution(
            std::span<T> data,
            Vector<usize, n> const& dimensions,
            Vector<T, n> const& low,
            Vector<T, n> const& high
        ) noexcept {
            integral = T{0};
            this->low = low[0];
            this->high = high[0];

            dim = dimensions[0];
            rows.resize(dim);
            cdf.resize(dim + 1);
            auto size = math::prod(dimensions) / dim;

            for (auto i = 0uz; i < dim; i++) {
                if constexpr (n == 1uz) {
                    rows[i] = math::abs(data[i]);
                    cdf[i + 1] = rows[i];
                } else {
                    rows[i] = Element{std::span{&data[i * size], size}, cut(dimensions), cut(low), cut(high)};
                    cdf[i + 1] = rows[i].integral;
                }
                cdf[i + 1] += cdf[i];
            }

            integral = cdf.back();
            for (auto i = 1uz; i <= dim; i++) {
                if (integral == 0.f) {
                    cdf[i] = T(i) / T(dim);
                } else {
                    cdf[i] /= integral;
                }
            }
        }

        auto sample(Vector<T, n> const& u) const noexcept -> Vector<T, n> {
            auto idx = 1uz;
            for (; idx < dim && cdf[idx] <= u[0]; idx++) {}
            idx--;

            auto t = guarded_div(u[0] - cdf[idx], cdf[idx + 1uz] - cdf[idx]);
            auto p = math::lerp(low, high, (T(idx) + t) / T(dim));

            if constexpr (n == 1uz) {
                return {p};
            } else {
                return math::consume(rows[idx].sample(cut(u)), p);
            }
        }

        auto pdf(Vector<T, n> const& p) const noexcept -> T {
            auto idx = std::clamp(
                usize((p[0] - low) / (high - low) * T(dim)),
                0uz, dim - 1uz
            );
            auto prob = (cdf[idx + 1] - cdf[idx]) / ((high - low) / T(dim));
            if constexpr (n == 1uz) {
                return prob;
            } else {
                return rows[idx].pdf(cut(p)) * prob;
            }
        }

    private:
        std::vector<Element> rows{};
        std::vector<T> cdf{};

        usize dim;
        T low{};
        T high{};
        T integral{};

        template<typename U, usize m>
        requires std::floating_point<U>
        friend struct Piecewise_Distribution;
    };
}
