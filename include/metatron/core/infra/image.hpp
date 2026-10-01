#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/stl/array.hpp>

namespace mtt:: inline infra {

namespace fmt {

namespace detail { template<u8... bits> using seq = std::integer_sequence<u8, bits...>; }

enum struct encoding: u8 { sf, uf, un, sn, ui, si, srgb, ldr, hdr };

template<u8... bits>
requires (true
&& ((sizeof...(bits) == 1 || sizeof...(bits) == 2 || sizeof...(bits) == 4)
&& (((bits == 8) && ...) || ((bits == 16) && ...) || ((bits == 32) && ...)))
|| std::same_as<detail::seq<bits...>, detail::seq<5, 6, 5>>
|| std::same_as<detail::seq<bits...>, detail::seq<4, 4, 4, 4>>
|| std::same_as<detail::seq<bits...>, detail::seq<5, 5, 5, 1>>
|| std::same_as<detail::seq<bits...>, detail::seq<10, 10, 10, 2>>
|| std::same_as<detail::seq<bits...>, detail::seq<11, 11, 10>>
|| std::same_as<detail::seq<bits...>, detail::seq<9, 9, 9, 5>>)
struct ordinary final {
    template<encoding e>
    auto static constexpr encodable = e <= encoding::srgb
    && (e != encoding::srgb || ((bits == 8) && ...))
    && (e != encoding::sn || ((bits % 8 == 0 && bits <= 16) && ...))
    && (e != encoding::sf || ((bits % 8 == 0 && bits >= 16) && ...))
    && (e != encoding::si || ((bits % 8 == 0) && ...))
    && (e != encoding::ui || ((bits % 8 == 0) && ...) || std::same_as<detail::seq<bits...>, detail::seq<10, 10, 10, 2>>)
    && (e != encoding::uf || std::same_as<detail::seq<bits...>, detail::seq<11, 11, 10>> || std::same_as<detail::seq<bits...>, detail::seq<9, 9, 9, 5>>)
    && (e != encoding::un || (((bits < 32) && ...) && !std::same_as<detail::seq<bits...>, detail::seq<11, 11, 10>> && !std::same_as<detail::seq<bits...>, detail::seq<9, 9, 9, 5>>));
    auto static constexpr channels = bv<sizeof...(bits)>{bits...};
    auto static constexpr block = bv2{1, 1};
    auto static constexpr bytes = (usize{bits} + ...) / 8;
};

template<u8 w, u8 h>
requires (false
|| (w == 4 && h == 4)
|| (w == 5 && (h == 4 || h == 5))
|| (w == 6 && (h == 5 || h == 6))
|| (w == 8 && (h == 5 || h == 6 || h == 8))
|| (w == 10 && (h == 5 || h == 6 || h == 8 || h == 10))
|| (w == 12 && (h == 10 || h == 12)))
struct astc final {
    template<encoding e>
    auto static constexpr encodable = e >= encoding::srgb;
    auto static constexpr block = bv2{w, h};
    auto static constexpr bytes = usize{16};
};

template<u8 n>
requires (n >= 1 && n <= 7)
struct bc final {
    template<encoding e>
    auto static constexpr encodable = false
    || ((n == 1 || n == 2 || n == 3 || n == 7) && (e == encoding::un || e == encoding::srgb))
    || ((n == 4 || n == 5) && (e == encoding::un || e == encoding::sn))
    || (n == 6 && (e == encoding::uf || e == encoding::sf));
    auto static constexpr block = bv2{4, 4};
    auto static constexpr bytes = usize{n == 1 || n == 4 ? 8 : 16};
};

}

template<typename R, usize dims, usize usage>
requires (stl::array<fv4, iv4, uv4>::contains<R> && (dims == 2 || dims == 3) && usage > 0 && usage < 4)
struct image final {
    auto static constexpr sampled = 1uz << 0;
    auto static constexpr storage = 1uz << 1;

    struct descriptor final { uzv3 size; };

    template<typename S, fmt::encoding E>
    requires (true
    && S::template encodable<E>
    && (E == fmt::encoding::ui ? std::same_as<R, uv4> : E == fmt::encoding::si ? std::same_as<R, iv4> : std::same_as<R, fv4>)
    && ((usage & storage) == 0 || (requires { S::channels; } && E != fmt::encoding::srgb && S::channels[0] % 8 == 0))
    && (dims != 3 || requires { S::channels; }))
    auto update(descriptor const& desc) noexcept -> void;

    auto operator[](fv2 uv, fv4 duvdxy) const noexcept -> R requires (std::same_as<R, fv4> && (usage & sampled) != 0 && dims == 2);
    auto operator[](fv3 uvw) const noexcept -> R requires (std::same_as<R, fv4> && (usage & sampled) != 0);
    auto operator[](uzv3 xyz) noexcept -> R& requires ((usage & storage) != 0);
    auto operator[](uzv3 xyz) const noexcept -> R;
    operator u32() const noexcept { return idx; }
    operator bool() const noexcept { return idx != math::maxv<u32>; }

    u32 idx = math::maxv<u32>;
};

template<usize dims> using ft = image<fv4, dims, 1>;
template<usize dims> using ftw = image<fv4, dims, 3>;
template<usize dims> using frw = image<fv4, dims, 2>;
template<usize dims> using irw = image<iv4, dims, 2>;
template<usize dims> using urw = image<uv4, dims, 2>;

using ft2 = ft<2>;
using ft3 = ft<3>;
using ftw2 = ftw<2>;
using ftw3 = ftw<3>;
using frw2 = frw<2>;
using frw3 = frw<3>;
using irw2 = irw<2>;
using irw3 = irw<3>;
using urw2 = urw<2>;
using urw3 = urw<3>;

}
