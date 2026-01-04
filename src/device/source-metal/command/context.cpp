#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include "context.hpp"
#include <metatron/core/stl/filesystem.hpp>

namespace mtt {
    auto to_mtl(std::string_view str) noexcept -> mut<NS::String> {
        return NS::String::string(str.data(), NS::UTF8StringEncoding);
    }
}

namespace mtt::command {
    Context::Impl::Impl() noexcept {
        device = MTL::CreateSystemDefaultDevice();
        queue = device->newCommandQueue();

        auto resc = MTL::ResidencySetDescriptor::alloc()->init();
        resc->setInitialCapacity(16);
        MTT_MTL_GUARD(residency = device->newResidencySet(resc, &err));
        queue->addResidencySet(residency.get());

        auto opt = stl::filesystem::hit("pipeline.bin");
        auto besc = MTL::BinaryArchiveDescriptor::alloc()->init();
        if (opt) besc->setUrl(NS::URL::fileURLWithPath(to_mtl(opt->string())));
        MTT_MTL_GUARD(archive = device->newBinaryArchive(besc, &err));
    }

    Context::Impl::~Impl() noexcept {
        auto path = stl::filesystem::instance().cache / "pipeline.bin";
        MTT_MTL_GUARD(archive->serializeToURL(NS::URL::fileURLWithPath(to_mtl(path.string())), &err));
    }

    auto Context::init() noexcept -> void {
        Context::instance();
    }

    auto guard(mut<NS::Error> err) noexcept -> void {
        if (err->code() != NS::Integer{0}) stl::abort("metal error: {}"
        , err->localizedDescription()->cString(NS::UTF8StringEncoding));
    }
}
