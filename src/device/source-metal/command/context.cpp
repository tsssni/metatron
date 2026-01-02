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
        auto opt = stl::filesystem::hit("pipeline.bin");
        auto desc = MTL::BinaryArchiveDescriptor::alloc()->init();
        if (opt) desc->setUrl(NS::URL::fileURLWithPath(to_mtl(opt->string())));
        MTT_MTL_GUARD(archive = device->newBinaryArchive(desc, &err));
        desc->release();
    }

    Context::Impl::~Impl() noexcept {
        auto path = stl::filesystem::instance().cache / "pipeline.bin";
        MTT_MTL_GUARD(archive->serializeToURL(NS::URL::fileURLWithPath(to_mtl(path.string())), &err));
    }

    auto Context::init() noexcept -> void {
        Context::instance();
    }

    auto guard(mut<NS::Error> err) noexcept -> void {
        if (err->code() != NS::Integer{0}) {
            auto desc = err->localizedDescription();
            auto reason = err->localizedFailureReason();
            auto suggestion = err->localizedRecoverySuggestion();

            stl::abort("desc: {} reason: {} sugg: {}"
                , desc ? desc->cString(NS::UTF8StringEncoding) : "(none)"
                , reason ? reason->cString(NS::UTF8StringEncoding) : "(none)"
                , suggestion ? suggestion->cString(NS::UTF8StringEncoding) : "(none)"
            );
        }
    }
}
