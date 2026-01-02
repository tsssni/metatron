#include "pipeline.hpp"

namespace mtt::shader {
    Pipeline::Pipeline(cref<Descriptor> desc) noexcept {
        // TODO: building from source as apple sdk in nixpkgs does not provide metal compiler
        auto base_path = stl::path{"shader"} / desc.name;
        auto shader_path = stl::path{base_path}.concat(".metal");
        auto metal = stl::filesystem::load(stl::filesystem::find(shader_path), {}, true);
        auto src = to_mtl(std::string_view{view<char>(metal.data()), metal.size()});

        auto& ctx = command::Context::instance().impl;
        auto device = ctx->device.get();
        auto options = MTL::CompileOptions::alloc();
        options->setLanguageVersion(MTL::LanguageVersion3_2);
        MTT_MTL_GUARD(impl->library = device->newLibrary(src, options, &err));

        auto name = desc.name.substr(desc.name.find_first_of('.') + 1);
        impl->function = impl->library->newFunction(to_mtl(name));
        auto cesc = MTL::ComputePipelineDescriptor::alloc();
        cesc->setComputeFunction(impl->function.get());
        cesc->setBinaryArchives(NS::Array::array(ctx->archive.get()));
        cesc->setShaderValidation(MTL::ShaderValidationEnabled);
        MTT_MTL_GUARD(impl->pipeline = device->newComputePipelineState(cesc, MTL::PipelineOptionNone, nullptr, &err));
    }
}
