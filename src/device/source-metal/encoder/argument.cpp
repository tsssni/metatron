#include "argument.hpp"
#include "transfer.hpp"
#include "../shader/argument.hpp"
#include "../command/buffer.hpp"
#include "../opaque/accel.hpp"
#include "../opaque/buffer.hpp"
#include "../opaque/image.hpp"
#include "../opaque/grid.hpp"
#include "../opaque/sampler.hpp"
#include <metatron/core/math/bit.hpp>

namespace mtt::encoder {

    template<typename T>
    auto Argument_Encoder::Impl::identify(auto transform, T src, mut<shader::Argument> args, u32 base) noexcept -> void {
        auto ids = src
        | std::views::transform(transform)
        | std::ranges::to<std::vector<MTL::ResourceID>>();
        auto size = (u32)(src.size() * sizeof(MTL::ResourceID));
        std::memcpy(args->set->ptr + base, ids.data(), size);
        args->set->dirty.push_back({base, size});
    }

    Argument_Encoder::Argument_Encoder(mut<command::Buffer> cmd, mut<shader::Argument> args) noexcept:
    cmd(cmd), args(args) {}

    auto Argument_Encoder::submit() noexcept -> void {
        auto transfer = Transfer_Encoder{cmd};
        if (args->impl->parameters)
            transfer.upload(*args->impl->parameters);
        transfer.upload(*args->set);
        transfer.submit();
    }

    auto Argument_Encoder::push(std::span<byte const> set, uv2 range) noexcept -> void {
        using Type = shader::Descriptor::Type;

        auto begin = range[0];
        auto end = range[0] + range[1];
        auto first = std::ranges::upper_bound(
            args->reflection, begin, {}, &shader::Descriptor::offset
        ) - std::ranges::begin(args->reflection) - 1;

        for (auto i = first; i < args->reflection.size(); ++i) {
            auto& desc = args->reflection[i];
            if (desc.offset >= end) break;
            if (desc.count == 0) continue;
            auto src = set.data() + desc.offset;
            auto base = (u32)args->impl->offsets[i];

            switch (desc.type) {
            case Type::parameter: {
                auto lo = math::max(begin, desc.offset);
                auto hi = math::min(end, desc.offset + desc.size);
                auto offset = lo - desc.offset;
                auto size = hi - lo;
                std::memcpy(args->impl->parameters->ptr + offset, set.data() + lo, size);
                args->impl->parameters->dirty.push_back({offset, size});
                break;
            }
            case Type::sampler:
                impl->identify(+[](mut<opaque::Sampler> ptr) {
                    return ptr->impl->sampler->gpuResourceID();
                }, std::span{mut<mut<opaque::Sampler>>(src), desc.count}, args, base); break;
            case Type::accel:
                impl->identify(+[](mut<opaque::Acceleration> ptr) {
                    return ptr->impl->instances->gpuResourceID();
                }, std::span{mut<mut<opaque::Acceleration>>(src), desc.count}, args, base); break;
            case Type::image:
                impl->identify(+[](cref<opaque::Image::View> view) {
                    return view.ptr->impl->texture->gpuResourceID();
                }, std::span{mut<opaque::Image::View>(src), desc.count}, args, base); break;
            case Type::grid:
                impl->identify(+[](cref<opaque::Grid::View> view) {
                    return view.ptr->impl->texture->gpuResourceID();
                }, std::span{mut<opaque::Grid::View>(src), desc.count}, args, base); break;
            }
        }
    }

    auto Argument_Encoder::push(std::span<byte const> set, std::tuple<u32, std::span<byte const>> bindless) noexcept -> void {
        using Type = shader::Descriptor::Type;
        auto [offset, span] = bindless;
        auto last = args->reflection.size() - 1;
        auto& desc = args->reflection[last];
        auto base = (u32)(args->impl->offsets[last] + offset * sizeof(MTL::ResourceID));

        if (desc.type == Type::image) impl->identify(+[](cref<opaque::Image::View> view) {
            return view.ptr->impl->texture->gpuResourceID();
        }, std::span{mut<opaque::Image::View>(span.data()), span.size() / sizeof(opaque::Image::View)}, args, base);
        else impl->identify(+[](cref<opaque::Grid::View> view) {
            return view.ptr->impl->texture->gpuResourceID();
        }, std::span{mut<opaque::Grid::View>(span.data()), span.size() / sizeof(opaque::Grid::View)}, args, base);
    }
}
