#pragma once

namespace mtt::core::wave {

auto elect() noexcept -> bool;
template<typename T>
auto max(T value) noexcept -> T;
template<typename T>
auto sum(T value) noexcept -> T;

}
