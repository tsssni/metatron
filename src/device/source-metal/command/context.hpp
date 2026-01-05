#pragma once
#include <metatron/device/command/context.hpp>
#include <Metal/Metal.hpp>

namespace mtt {
    template<typename T>
    struct mtl final {
        mtl() noexcept: ptr(nullptr) {}
        mtl(mut<T> ptr) noexcept: ptr(ptr) {}
        mtl(rref<mtl> x) noexcept: ptr(x.ptr) { x.ptr = nullptr; }
        mtl(cref<mtl>) noexcept = delete;
        ~mtl() noexcept { if (ptr) ptr->release(); }
        auto operator=(rref<mtl> x) noexcept -> ref<mtl> {
            if (ptr) ptr->release();
            ptr = x.ptr;
            x.ptr = nullptr;
            return *this;
        }
        auto operator=(cref<mtl>) noexcept -> ref<mtl> = delete;
        auto operator->() noexcept -> mut<T> { return ptr; }
        auto operator->() const noexcept -> view<T> { return ptr; }
        operator bool() const noexcept { return ptr != nullptr; }
        auto get() noexcept -> mut<T> { return ptr; };
        auto get() const noexcept -> view<T> { return ptr; };
    private:
        mut<T> ptr;
    };

    auto to_mtl(std::string_view str) noexcept -> mut<NS::String>;
}

namespace mtt::command {
    struct Context::Impl final {
        mtl<MTL::Device> device;
        mtl<MTL::ResidencySet> residency;
        mtl<MTL::BinaryArchive> archive;
        Impl() noexcept;
        ~Impl() noexcept;
    };

    auto guard(mut<NS::Error> err) noexcept -> void;
    #define MTT_MTL_GUARD(x) {auto err = mut<NS::Error>{}; x; command::guard(err);}
}
