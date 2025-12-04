#include <metatron/network/wired/tcp.hpp>
#include <metatron/network/wired/socket.hpp>
#include <sys/socket.h>
#include <cstring>
#include <signal.h>
#include <netdb.h>
#include <unistd.h>

namespace mtt::wired {
    struct Tcp_Socket::Impl final {
        Address address;
        Socket socket{invalid_socket};

        Impl(cref<Address> address) noexcept: address(address) {
            ::signal(SIGPIPE, SIG_IGN);
        }

        ~Impl() {
            if (socket != invalid_socket) disconnect();
        }

        auto connect() noexcept -> void {
            auto hints = addrinfo{};
            auto* info = (addrinfo*)nullptr;
            hints.ai_family = PF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            if (auto err = ::getaddrinfo(
                address.host.c_str(), address.port.c_str(), &hints, &info
            )) stl::abort("getaddrinfo failed: {}", ::gai_strerror(err));

            for (auto* ptr = info; ptr; ptr = ptr->ai_next) {
                socket = ::socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
                if (socket == invalid_socket) continue;

                if (::connect(socket, ptr->ai_addr, ptr->ai_addrlen) == socket_error) {
                    ::close(socket);
                    socket = invalid_socket;
                    continue;
                }

                break;
            }

            ::freeaddrinfo(info);
            if (socket == invalid_socket)
                stl::print("tcp could not connect to {}", address);
        }

        auto disconnect() noexcept -> void {
            if (socket == invalid_socket) return;
            ::close(socket);
            socket = invalid_socket;
        }

        auto send(std::span<byte const> data) noexcept -> bool {
            if (socket == invalid_socket) {
                connect();
                if (socket == invalid_socket) return false;
            }

            auto sent_size = ::send(socket, view<char>(data.data()), data.size(), 0);
            if (sent_size == data.size()) return true;

            stl::print("send failed: {}", ::strerror(errno));
            disconnect();
            socket = invalid_socket;
            return false;
        }
    };

    Tcp_Socket::Tcp_Socket(cref<Address> address) noexcept
    : stl::capsule<Tcp_Socket>(address) {}

    auto Tcp_Socket::send(std::span<byte const> data) noexcept -> bool {
        return impl->send(data);
    }
}
