#include "debug.h"
#include "socket.h"

namespace fx {
namespace network {

typedef socket::error_code error_code;

using namespace std::chrono;
using namespace std::chrono_literals;

socket::~socket()
{
    socket::close();
}

error_code socket::get_recv_bufsize(size_t& size) const
{
    int n = 0;
    error_code err = get_option(SO_RCVBUF, SOL_SOCKET, n);
    size = static_cast<size_t>(n);
    return err;
}

error_code socket::get_send_bufsize(size_t& size) const
{
    int n = 0;
    error_code err = get_option(SO_SNDBUF, SOL_SOCKET, n);
    size = static_cast<size_t>(n);    
    return err;
}

error_code socket::set_recv_bufsize(size_t size)
{
    return set_option(SO_RCVBUF, SOL_SOCKET, static_cast<int>(size));
}

error_code socket::set_send_bufsize(size_t size)
{
    return set_option(SO_SNDBUF, SOL_SOCKET, static_cast<int>(size));
}

error_code socket::set_reuse_address(bool on_off)
{
    int opt = (on_off ? 1 : 0);
    return set_option(SO_REUSEADDR, SOL_SOCKET, opt);
}

int socket::get_socket_error()
{
    int err = -1;
    get_option(SO_ERROR, SOL_SOCKET, err);
    return err;
}

inet_address_ptr socket::get_socket_address() const
{
    inet_address_ptr addr_ptr;

    if (sock_ != bad_socket)
    {
        struct sockaddr_in6 sa6;
        socklen_type len = sizeof(sa6);
        sockaddr* psa = reinterpret_cast<sockaddr*>(&sa6);

        int rc = getsockname(sock_, psa, &len);

        if (rc == 0)
        {
            if (psa->sa_family == AF_INET)
            {
                addr_ptr = std::make_shared<inet4_address>(psa);
            }
            else if (psa->sa_family == AF_INET6)
            {
                addr_ptr = std::make_shared<inet6_address>(psa);
            }
        }
    }

    return addr_ptr;
}

inet_address_ptr socket::get_peer_address() const
{
    inet_address_ptr addr_ptr;

    if (sock_ != bad_socket)
    {
        struct sockaddr_in6 sa6;
        socklen_type len = sizeof(sa6);
        sockaddr* psa = reinterpret_cast<sockaddr*>(&sa6);

        int rc = getpeername(sock_, psa, &len);

        if (rc == 0)
        {
            if (psa->sa_family == AF_INET)
            {
                addr_ptr = std::make_shared<inet4_address>(psa);
            }
            else if (psa->sa_family == AF_INET6)
            {
                addr_ptr = std::make_shared<inet6_address>(psa);
            }
        }
    }

    return addr_ptr;
}

bool socket::is_readable() const
{
    return wait_for_readable(0ms) == error_code::success;
}

bool socket::is_writable() const
{
    return wait_for_writable(0ms) == error_code::success;
}

} // namespace network
} // namespace fx
