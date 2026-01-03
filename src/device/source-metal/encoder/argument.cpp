#include "argument.hpp"
#include "transfer.hpp"
#include "../command/buffer.hpp"
#include "../opaque/accel.hpp"
#include "../opaque/buffer.hpp"
#include "../opaque/image.hpp"
#include "../opaque/grid.hpp"
#include "../opaque/sampler.hpp"

namespace mtt::encoder {
    Argument_Encoder::Argument_Encoder(mut<command::Buffer> cmd, mut<shader::Argument> args) noexcept:
    cmd(cmd), args(args) {}

    auto Argument_Encoder::submit() noexcept -> void {}
    auto Argument_Encoder::upload() noexcept -> void {
        auto encoder = Transfer_Encoder{cmd};
        if (args->parameters) encoder.upload(*args->parameters);
        if (args->bindless) encoder.upload(*args->bindless);
        encoder.upload(*args->set);
        encoder.submit();
    }

    template<typename T>
    auto Argument_Encoder::Impl::bind(mut<Argument_Encoder> encoder, std::string_view field, view<T> resource) noexcept -> void {
        auto args = encoder->args;
        auto binding = encoder->args->index(field);
        auto size = sizeof(MTL::ResourceID);
        mut<MTL::ResourceID>(args->set->ptr)[binding] = resource->gpuResourceID();
        args->set->dirty.push_back({binding * size, size});
    }

    template<typename T>
    auto Argument_Encoder::Impl::bind(mut<Argument_Encoder> encoder, shader::Bindless<T> bindless) noexcept -> void {
        auto args = encoder->args;
        auto ids = bindless.list
        | std::views::transform([](auto view) { return view.ptr->impl->texture->gpuResourceID(); })
        | std::ranges::to<std::vector<MTL::ResourceID>>();
        auto size = sizeof(MTL::ResourceID);
        std::memcpy(args->bindless->ptr + bindless.offset * size, ids.data(), ids.size() * size);
        args->bindless->dirty.push_back({bindless.offset * size, ids.size() * size});
    }

    auto Argument_Encoder::bind(std::string_view field, view<opaque::Acceleration> accel) noexcept -> void { impl->bind<MTL::AccelerationStructure>(this, field, accel->impl->instances.get()); }
    auto Argument_Encoder::bind(std::string_view field, view<opaque::Sampler> sampler) noexcept -> void { impl->bind<MTL::SamplerState>(this, field, sampler->impl->sampler.get()); }
    auto Argument_Encoder::bind(std::string_view field, opaque::Image::View image) noexcept -> void { impl->bind<MTL::Texture>(this, field, image.ptr->impl->texture.get()); }
    auto Argument_Encoder::bind(std::string_view field, opaque::Grid::View grid) noexcept -> void {  impl->bind<MTL::Texture>(this, field, grid.ptr->impl->texture.get()); }
    auto Argument_Encoder::bind(std::string_view field, shader::Bindless<opaque::Image> images) noexcept -> void { impl->bind(this, images); }
    auto Argument_Encoder::bind(std::string_view field, shader::Bindless<opaque::Grid> grids) noexcept -> void { impl->bind(this, grids); }

    auto Argument_Encoder::acquire(std::string_view field, std::span<byte const> uniform) noexcept -> void {
        std::memcpy(args->parameters->ptr, uniform.data(), uniform.size());
        args->parameters->dirty.push_back({0, u32(uniform.size())});
    }

    auto Argument_Encoder::acquire(std::string_view field, opaque::Image::View image) noexcept -> void {}
    auto Argument_Encoder::acquire(std::string_view field, opaque::Grid::View grid) noexcept -> void {}
}
