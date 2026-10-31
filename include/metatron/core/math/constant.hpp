#pragma once
#include <limits>

namespace mtt::math {
    auto constexpr pi = 3.14159265358979f;
    template<typename T>
    auto constexpr piv = T(3.14159265358979323846);

    template<typename T>
    auto constexpr epsilon = std::numeric_limits<T>::epsilon();
    template<typename T>
    auto constexpr minv = std::numeric_limits<T>::min();
    template<typename T>
    auto constexpr maxv = std::numeric_limits<T>::max();
    template<typename T>
    auto constexpr high = std::numeric_limits<T>::max();
    template<typename T>
    auto constexpr low = std::numeric_limits<T>::lowest();
    template<typename T>
    auto constexpr inf = std::numeric_limits<T>::infinity();

    template<>
    auto constexpr epsilon<f16> = (f16)0x1p-10f;
    template<>
    auto constexpr minv<f16> = (f16)0x1p-14f;
    template<>
    auto constexpr maxv<f16> = (f16)65504.f;
    template<>
    auto constexpr high<f16> = maxv<f16>;
    template<>
    auto constexpr low<f16> = -maxv<f16>;
    template<>
    auto constexpr inf<f16> = (f16)__builtin_inff();
}
