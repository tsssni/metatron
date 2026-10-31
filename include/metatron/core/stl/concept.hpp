#pragma once
#include <concepts>

namespace mtt::stl {

using std::same_as;
using std::convertible_to;
using std::totally_ordered;
using std::constructible_from;
using std::invocable;
using std::integral;
using std::unsigned_integral;

// for half
template<typename T>
concept floating_point = __is_floating_point(T);

}
