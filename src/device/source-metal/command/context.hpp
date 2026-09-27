#pragma once
#include <metatron/device/command/context.hpp>
#include <Metal/Metal.hpp>
#include <string_view>
#undef nil
#undef Nil

namespace mtt {
    template<typename T>
    struct mtl final {
        mtl() noexcept: ptr(nullptr) {}
        mtl(T* ptr) noexcept: ptr(ptr) {}
        mtl(mtl&& x) noexcept: ptr(x.ptr) { x.ptr = nullptr; }
        mtl(mtl const&) noexcept = delete;
        ~mtl() noexcept { if (ptr) ptr->release(); }
        auto operator=(mtl&& x) noexcept -> mtl& {
            if (ptr) ptr->release();
            ptr = x.ptr;
            x.ptr = nullptr;
            return *this;
        }
        auto operator=(mtl const&) noexcept -> mtl& = delete;
        auto operator->() noexcept -> T* { return ptr; }
        auto operator->() const noexcept -> T const* { return ptr; }
        operator bool() const noexcept { return ptr != nullptr; }
        auto get() noexcept -> T* { return ptr; };
        auto get() const noexcept -> T const* { return ptr; };
    private:
        T* ptr;
    };

    auto to_mtl(std::string_view str) noexcept -> NS::String*;
}

namespace mtt::command {
    struct Context::Impl final {
        mtl<MTL::Device> device;
        mtl<MTL::ResidencySet> residency;
        mtl<MTL::BinaryArchive> archive;
        Impl() noexcept;
        ~Impl() noexcept;
    };

    auto guard(NS::Error* err) noexcept -> void;
    #define MTT_MTL_GUARD(x) {auto err = (NS::Error*)nullptr; x; command::guard(err);}
}
