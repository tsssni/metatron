#pragma once
#include <string>
#include <span>
#include <vector>
#include <cstdint>

namespace mtt::wired {
    struct Packet final {
        std::vector<byte> payload;

        template<typename... Args>
        auto emplace(Args... args) noexcept -> void {
            auto constexpr size = (sizeof(args) + ...);
            auto offset = payload.size();
            payload.resize(payload.size() + size);
            ((*mut<Args>(&payload[offset]) = args, offset += sizeof(Args)), ...);
        }

        template<typename T>
        auto emplace(std::span<T> span) noexcept -> void {
            if constexpr (std::is_same_v<T, std::string_view>) {
                for (auto str: span) emplace(str);
            } else {
                auto size = span.size() * sizeof(T);
                auto offset = payload.size();
                payload.resize(payload.size() + size);
                std::memcpy(&payload[offset], span.data(), size);
            }
        }

        auto emplace(std::string_view str) noexcept -> void {
            auto size = str.size();
            auto offset = payload.size();
            payload.resize(payload.size() + size + 1);
            std::memcpy(&payload[offset], str.data(), size);
            payload[offset + size] = '\0';
        }
    };

}
