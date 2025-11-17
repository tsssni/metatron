#pragma once
#include <metatron/core/stl/singleton.hpp>
#include <metatron/core/stl/print.hpp>
#include <metatron/core/math/constant.hpp>
#include <cstring>
#include <cassert>
#include <mutex>

namespace mtt::stl {
    struct stack final: singleton<stack> {
        stack() noexcept:
        storage(nullptr), length(0), capacity(0),
        mutex(make_obj<std::mutex>()) {}
        ~stack() noexcept { if (storage) std::free(storage); }

        auto alloc(uptr length) noexcept -> uptr {
            if (this->length + length > capacity) grow();
            auto offset = this->length;
            this->length += length;
            return offset;
        }

        auto data() noexcept -> mut<byte> { return storage; };
        auto data() const noexcept -> view<byte> { return storage; };
        auto size() const noexcept -> uptr { return length; }
        auto lock() const noexcept {return std::unique_lock{*mutex};}

    private:
        auto grow() noexcept -> void {
            auto storage = mut<byte>(std::malloc(capacity + block_size));
            if (!storage) {
                std::println("arena run out of memory");
                std::abort();
            }

            if (this->storage) {
                std::memcpy(storage, this->storage, capacity);
                std::free(this->storage);
            }
            this->storage = storage;
            capacity += block_size;
        }

        auto static constexpr block_size = (1ull << 28);

        mut<byte> storage;
        uptr length;
        uptr capacity;
        obj<std::mutex> mutex;
    };
}

namespace mtt {
    template<typename T>
    struct buf final {
        buf() noexcept: ptr(math::maxv<uptr>), length(math::maxv<usize>) {};
        buf(cref<buf>) noexcept = default;
        buf(rref<buf>) noexcept = default;
        auto operator=(cref<buf>) noexcept -> ref<buf> = default;
        auto operator=(rref<buf>) noexcept -> ref<buf> = default;

        buf(std::span<T> range) noexcept {
            auto size = range.size() * sizeof(T);
            ptr = stl::stack::instance().alloc(size);
            length = range.size();
            std::memcpy(data(), range.data(), size);
        }

        operator std::span<T>() noexcept { return {data(), size()}; }
        operator std::span<T const>() const noexcept { return {data(), size()}; }

        auto data() noexcept -> mut<T> { return (mut<T>)(stl::stack::instance().data() + ptr); };
        auto data() const noexcept -> view<T> { return (view<T>)(stl::stack::instance().data() + ptr); };
        auto size() const noexcept -> usize { return length; }
        auto empty() const noexcept -> bool { return length == 0; }
        auto operator[](usize i) noexcept -> ref<T> { return data()[i]; }
        auto operator[](usize i) const noexcept -> cref<T> { return data()[i]; }

        auto subbuf(usize i, usize size) -> buf<T> {
            auto buf = *this;
            buf.ptr += i * sizeof(T);
            buf.length = size;
            return buf;
        }

    private:
        uptr ptr;
        usize length;
    };
}
