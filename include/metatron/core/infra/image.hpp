#pragma once
#include <metatron/core/infra/vector.hpp>
#include <metatron/core/stl/array.hpp>
#include <metatron/core/stl/concept.hpp>
#include <utility>

namespace mtt::core {

enum struct encoding : u8 {
    sf,
    uf,
    un,
    sn,
    ui,
    si,
    srgb,
    ldr,
    hdr,
};

struct format final {
    template<u8... ns>
    using seq = std::integer_sequence<u8, ns...>;
};

template<u8... bits>
requires([] {
    using seq = format::seq<bits...>;
    auto constexpr uniform = true
        && (sizeof...(bits) == 1 || sizeof...(bits) == 2 || sizeof...(bits) == 4)
        && (((bits == 8) && ...) || ((bits == 16) && ...) || ((bits == 32) && ...));
    auto constexpr packed = false
        || std::same_as<seq, format::seq<5, 6, 5>>
        || std::same_as<seq, format::seq<4, 4, 4, 4>>
        || std::same_as<seq, format::seq<5, 5, 5, 1>>
        || std::same_as<seq, format::seq<10, 10, 10, 2>>
        || std::same_as<seq, format::seq<11, 11, 10>>
        || std::same_as<seq, format::seq<9, 9, 9, 5>>;
    return uniform || packed;
}())
struct ordinary final {
    using seq = format::seq<bits...>;

    template<encoding e>
    auto static constexpr encodable = e <= encoding::srgb
        && (e != encoding::srgb || ((bits == 8) && ...))
        && (e != encoding::sn || ((bits % 8 == 0 && bits <= 16) && ...))
        && (e != encoding::sf || ((bits % 8 == 0 && bits >= 16) && ...))
        && (e != encoding::si || ((bits % 8 == 0) && ...))
        && (e != encoding::ui || ((bits % 8 == 0) && ...) || std::same_as<seq, format::seq<10, 10, 10, 2>>)
        && (e != encoding::uf || std::same_as<seq, format::seq<11, 11, 10>> || std::same_as<seq, format::seq<9, 9, 9, 5>>)
        && (e != encoding::un || (((bits < 32) && ...) && !std::same_as<seq, format::seq<11, 11, 10>> && !std::same_as<seq, format::seq<9, 9, 9, 5>>));
    auto static constexpr channels = bv<sizeof...(bits)>{bits...};
    auto static constexpr block = bv2{1, 1};
    auto static constexpr bytes = (usize{bits} + ...) / 8;
};

template<u8 w, u8 h>
requires([] {
    return false
        || (w == 4 && h == 4)
        || (w == 5 && (h == 4 || h == 5))
        || (w == 6 && (h == 5 || h == 6))
        || (w == 8 && (h == 5 || h == 6 || h == 8))
        || (w == 10 && (h == 5 || h == 6 || h == 8 || h == 10))
        || (w == 12 && (h == 10 || h == 12));
}())
struct astc final {
    template<encoding e>
    auto static constexpr encodable = e >= encoding::srgb;
    auto static constexpr block = bv2{w, h};
    auto static constexpr bytes = usize{16};
};

template<u8 n>
requires(n >= 1 && n <= 7)
struct bc final {
    template<encoding e>
    auto static constexpr encodable = false
        || ((n == 1 || n == 2 || n == 3 || n == 7) && (e == encoding::un || e == encoding::srgb))
        || ((n == 4 || n == 5) && (e == encoding::un || e == encoding::sn))
        || (n == 6 && (e == encoding::uf || e == encoding::sf));
    auto static constexpr block = bv2{4, 4};
    auto static constexpr bytes = usize{n == 1 || n == 4 ? 8 : 16};
};

template<typename S, encoding E, typename R, usize dims, usize usage, bool unique>
concept encodable = [] {
    auto constexpr storage = 1uz << 1;
    if constexpr (!requires { requires S::template encodable<E>; }) return false;
    else {
        auto constexpr ui = E == encoding::ui && std::same_as<R, uv4>;
        auto constexpr si = E == encoding::si && std::same_as<R, iv4>;
        auto constexpr f = E != encoding::ui && E != encoding::si && std::same_as<R, fv4>;
        auto constexpr compat = ui || si || f;
        if constexpr (requires { S::channels; }) {
            auto constexpr writable = (usage & storage) == 0 || (E != encoding::srgb && S::channels[0] % 8 == 0);
            return unique && compat && writable;
        } else return unique && compat && (usage & storage) == 0 && dims != 3;
    }
}();

template<typename R, usize dims, usize usage, bool unique>
requires(array<fv4, iv4, uv4>::contains<R> && (dims == 2 || dims == 3) && usage > 0 && usage < 4)
struct image final {
    auto static constexpr sampled = 1uz << 0;
    auto static constexpr storage = 1uz << 1;

    u32 idx = maxv<u32>;
    image() noexcept = default;
    image(image const&) noexcept requires(!unique) = default;
    auto operator=(image const&) noexcept -> image& requires(!unique) = default;
    ~image() noexcept requires(!unique) = default;
    template<bool rhs_unique>
    image(image<R, dims, usage, rhs_unique> const& rhs) noexcept requires(!unique && rhs_unique);

    image(image const&) requires(unique) = delete;
    auto operator=(image const&) -> image& requires(unique) = delete;
    image(image&& rhs) noexcept requires(unique);
    auto operator=(image&& rhs) noexcept -> image& requires(unique);
    ~image() noexcept requires(unique);
    auto destroy() noexcept -> void requires(unique);

    template<typename S, encoding E>
    auto update(uzv4 size) noexcept -> void requires encodable<S, E, R, dims, usage, unique>;

    auto size(i32 mip = -1) const noexcept -> uzv4;
    auto operator[](fv2 uv, fv4 duvdxy) const noexcept -> R requires(std::same_as<R, fv4> && (usage & sampled) != 0 && dims == 2);
    auto operator[](fv3 uvw) const noexcept -> R requires(std::same_as<R, fv4> && (usage & sampled) != 0);
    auto operator[](uzv3 xyz) noexcept -> R& requires((usage & storage) != 0);
    auto operator[](uzv3 xyz) const noexcept -> R;
    operator u32() const noexcept;
    operator bool() const noexcept;
};

template<typename R, usize dims, usize usage, bool unique>
requires(array<fv4, iv4, uv4>::contains<R> && (dims == 2 || dims == 3) && usage > 0 && usage < 4)
template<bool rhs_unique>
image<R, dims, usage, unique>::image(image<R, dims, usage, rhs_unique> const& rhs) noexcept requires(!unique && rhs_unique)
: idx(rhs.idx) {}

template<typename R, usize dims, usize usage, bool unique>
requires(array<fv4, iv4, uv4>::contains<R> && (dims == 2 || dims == 3) && usage > 0 && usage < 4)
image<R, dims, usage, unique>::image(image&& rhs) noexcept requires(unique)
: idx(std::exchange(rhs.idx, maxv<u32>)) {}

template<typename R, usize dims, usize usage, bool unique>
requires(array<fv4, iv4, uv4>::contains<R> && (dims == 2 || dims == 3) && usage > 0 && usage < 4)
auto image<R, dims, usage, unique>::operator=(image&& rhs) noexcept -> image& requires(unique) {
    auto next = std::exchange(rhs.idx, maxv<u32>);
    destroy();
    idx = next;
    return *this;
}

template<typename R, usize dims, usize usage, bool unique>
requires(array<fv4, iv4, uv4>::contains<R> && (dims == 2 || dims == 3) && usage > 0 && usage < 4)
image<R, dims, usage, unique>::~image() noexcept requires(unique) { destroy(); }

template<typename R, usize dims, usize usage, bool unique>
requires(array<fv4, iv4, uv4>::contains<R> && (dims == 2 || dims == 3) && usage > 0 && usage < 4)
image<R, dims, usage, unique>::operator u32() const noexcept { return idx; }

template<typename R, usize dims, usize usage, bool unique>
requires(array<fv4, iv4, uv4>::contains<R> && (dims == 2 || dims == 3) && usage > 0 && usage < 4)
image<R, dims, usage, unique>::operator bool() const noexcept { return idx != maxv<u32>; }

template<typename I>
concept tiled = requires {
    []<typename R, usize dims, usize usage, bool unique>(image<R, dims, usage, unique> const*) {}((I const*)nullptr);
};

}

namespace mtt {

template<usize dims, bool unique>
using ft = core::image<fv4, dims, 1, unique>;
template<usize dims, bool unique>
using ftw = core::image<fv4, dims, 3, unique>;
template<usize dims, bool unique>
using frw = core::image<fv4, dims, 2, unique>;
template<usize dims, bool unique>
using irw = core::image<iv4, dims, 2, unique>;
template<usize dims, bool unique>
using urw = core::image<uv4, dims, 2, unique>;

using ft2 = ft<2, true>;
using ft3 = ft<3, true>;
using ftw2 = ftw<2, true>;
using ftw3 = ftw<3, true>;
using frw2 = frw<2, true>;
using frw3 = frw<3, true>;
using irw2 = irw<2, true>;
using irw3 = irw<3, true>;
using urw2 = urw<2, true>;
using urw3 = urw<3, true>;

using ft2v = ft<2, false>;
using ft3v = ft<3, false>;
using ftw2v = ftw<2, false>;
using ftw3v = ftw<3, false>;
using frw2v = frw<2, false>;
using frw3v = frw<3, false>;
using irw2v = irw<2, false>;
using irw3v = irw<3, false>;
using urw2v = urw<2, false>;
using urw3v = urw<3, false>;

}
