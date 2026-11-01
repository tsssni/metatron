#pragma once
#include <concepts>

namespace mtt::core {

// for half
template<typename T>
concept floating_point = __is_floating_point(T);

}
