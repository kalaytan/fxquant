#if defined(_WIN32)

#include "debug.h"
#include "socket.h"

namespace fx {
namespace network {

typedef socket::error_code error_code;

void socket::close()
{
    if (sock_ != bad_socket)
    {
        ::closesocket(sock_);
        sock_ = bad_socket;
    }
}

error_code socket::set_blocking_mode(bool on_off)
{
    if (sock_ == bad_socket)
    {
        return error_code::invalid_socket;
    }

    u_long arg = (on_off ? 0 : 1);
    int rc = ioctlsocket(sock_, FIONBIO, &arg);

    return (rc == 0) ? error_code::success : error_code::unspecified;
}

error_code socket::wait_for_readable(timeout_type to) const
{
    if (sock_ == bad_socket)
    {
        return error_code::invalid_socket;
    }

    fd_set set;
    FD_ZERO(&set);
    FD_SET(sock_, &set);

    // prepare timeout value
    struct timeval tv;
    tv.tv_sec = static_cast<long>(to.count() / 1000);
    tv.tv_usec = (to.count() % 1000) * 1000;

    int rc = select((int)sock_ + 1, &set, NULL, NULL, &tv);

    if (rc > 0)
    {
        return error_code::success;
    }

    if (rc == 0)
    {
        return error_code::timeout;
    }

    return error_code::unspecified;
}

error_code socket::wait_for_readable() const
{
    if (sock_ == bad_socket)
    {
        return error_code::invalid_socket;
    }

    fd_set set;
    FD_ZERO(&set);
    FD_SET(sock_, &set);

    int rc = select((int)sock_ + 1, &set, NULL, NULL, NULL);
    return (rc > 0) ? error_code::success : error_code::unspecified;
}

error_code socket::wait_for_writable(timeout_type to) const
{
    if (sock_ == bad_socket)
    {
        return error_code::invalid_socket;
    }

    fd_set set;
    FD_ZERO(&set);
    FD_SET(sock_, &set);

    // prepare timeout value
    struct timeval tv;
    tv.tv_sec = static_cast<long>(to.count() / 1000);
    tv.tv_usec = (to.count() % 1000) * 1000;

    int rc = ::select((int)sock_ + 1, NULL, &set, NULL, &tv);

    if (rc > 0)
    {
        return error_code::success;
    }

    if (rc == 0)
    {
        return error_code::timeout;
    }

    return error_code::unspecified;
}

error_code socket::wait_for_writable() const
{
    if (sock_ == bad_socket)
    {
        return error_code::invalid_socket;
    }

    fd_set set;
    FD_ZERO(&set);
    FD_SET(sock_, &set);

    int rc = ::select((int)sock_ + 1, NULL, &set, NULL, NULL);
    return (rc > 0) ? error_code::success : error_code::unspecified;
}

error_code socket::bind(const inet_address& addr)
{
    if (sock_ == bad_socket)
    {
        return error_code::invalid_socket;
    }

    int rc = ::bind(sock_, (const sockaddr*)addr,
        static_cast<int>(addr.size()));

    if (rc == SOCKET_ERROR)
    {
        int err = ::WSAGetLastError();

        if (err == WSAEINVAL)
        {
            // the socket is already bound to an address
            return error_code::already_bound;
        }

        return error_code::unspecified;
    }

    return error_code::success;
}

} // namespace network
} // namespace fx

#endif
