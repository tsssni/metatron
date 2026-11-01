#pragma once
#include <vector>

namespace mtt::core {

template<usize n>
struct Discrete_Distribution final {
    std::array<f32, n> pdf{0.f};
    std::array<f32, n + 1> cdf{0.f};
    auto update(std::array<f32, n> const& ws) noexcept -> void;
    auto sample(f32 u) const noexcept -> usize;
};

template<usize n>
auto Discrete_Distribution<n>::update(std::array<f32, n> const& ws) noexcept -> void {
    pdf = ws;
    cdf[0] = 0.f;
    for (auto i = 1; i <= n; ++i)
        cdf[i] = cdf[i - 1] + pdf[i - 1];
}

template<usize n>
auto Discrete_Distribution<n>::sample(f32 u) const noexcept -> usize {
    auto i = 0uz;
    for (; i < pdf.size() && u > cdf[i + 1]; ++i);
    return i;
}

struct Array_Distribution final {
    buf<f32> pdf;
    buf<f32> cdf;
    usize count = 0;
    auto update(std::span<f32> ws) noexcept -> void;
    auto sample(f32 u) const noexcept -> usize;
};

auto inline Array_Distribution::update(std::span<f32> ws) noexcept -> void {
    count = ws.size();
    this->pdf = count;
    this->cdf = count + 1;
    auto pdf = std::vector<f32>{ws.begin(), ws.end()};
    auto cdf = std::vector<f32>(ws.size() + 1);
    cdf[0] = 0.f;
    for (auto i = 1; i <= ws.size(); ++i)
        cdf[i] = cdf[i - 1] + pdf[i - 1];
    for (auto i = 0; i < ws.size(); ++i)
        pdf[i] /= cdf[ws.size()];
    this->pdf.map();
    this->cdf.map();
    for (auto i = 0uz; i < count; ++i) this->pdf[i] = pdf[i];
    for (auto i = 0uz; i <= count; ++i) this->cdf[i] = cdf[i];
    this->pdf.unmap();
    this->cdf.unmap();
}

auto inline Array_Distribution::sample(f32 u) const noexcept -> usize {
    auto i = 0uz;
    for (; i < count && u > cdf[i + 1]; ++i);
    return i;
}

}
