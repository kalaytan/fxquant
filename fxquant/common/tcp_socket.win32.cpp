#if defined(_WIN32)

#include <chrono>
#include "debug.h"
#include "socket.h"

using namespace std::chrono;

namespace fx {
namespace network {

typedef socket::error_code error_code;

error_code tcp_socket::connect(const inet_address& addr, timeout_type to)
{
    if (sock_ == bad_socket)
    {
        return error_code::invalid_socket;
    }

    // switch socket to non-blocking mode
    error_code err = set_blocking_mode(false);

    if (err != error_code::success)
    {
        return err;
    }

    int rc = ::connect(sock_, (const sockaddr*)addr,
        static_cast<int>(addr.size()));

    if (rc == 0)
    {
        return error_code::success; // connected
    }

    int winerr = ::WSAGetLastError();

    switch (winerr)
    {
    case WSAEISCONN:
        // the socket is already connected
        err = error_code::already_connected;
        break;

    case WSAEINTR:
        // the blocking call was canceled through
        err = error_code::canceled;
        break;

    case WSAECONNREFUSED:
        // the attempt to connect was forcefully rejected
        err = error_code::connection_refused;
        break;

    case WSAENETUNREACH:
        // the network cannot be reached from this host at this time
        err = error_code::network_unreached;
        break;

    case WSAEINPROGRESS:
    case WSAEWOULDBLOCK:
    case WSAEALREADY:
    {
        // The socket is non-blocking and the connection cannot be
        // completed immediately. After select() indicates writability,
        // use getsockopt() to read the SO_ERROR to determine whether
        // connect() completed successfully (SO_ERROR is zero)

        // wait for a connection to be established
        err = wait_for_writable(to);

        if (err != error_code::success)
        {
            break; // error
        }

        // get error code
        int sockerr = get_socket_error();

        if (sockerr != 0)
        {
            err = error_code::unspecified;
            break;
        }

        connected_ = true;
        err = error_code::success; // connected
    }
    break;

    default:
        err = error_code::unspecified;
    }

    return err;
}

error_code tcp_socket::read(void* buf, size_t buf_size, size_t& out_size, bool peek)
{
    DEBUG_ASSERT(buf && (buf_size > 0));

    out_size = 0;

    if (sock_ == bad_socket)
    {
        return error_code::invalid_socket;
    }

    if (!buf || (buf_size == 0))
    {
        return error_code::invalid_argument;
    }

    if (!is_readable())
    {
        return error_code::success; // no data is available
    }

    // The receive calls normally return any data available, up to the requested amount,
    // rather than waiting for receipt of the full amount requested.
    int rc = ::recv(sock_, reinterpret_cast<char*>(buf), static_cast<int>(buf_size), peek ? MSG_PEEK : 0);

    if (rc > 0)
    {
        out_size = static_cast<size_t>(rc);
        return error_code::success;
    }

    if (rc == 0)
    {
        // connection has been gracefully closed
        return error_code::connection_closed;
    }

    int last_err = ::WSAGetLastError();

    switch (last_err)
    {
    case WSAEWOULDBLOCK:
        return error_code::retry;
    case WSAENOTCONN:
        return error_code::not_connected;
    case WSAECONNABORTED:
    case WSAECONNRESET:
        return error_code::connection_closed;
    }

    return error_code::unspecified;
}

error_code tcp_socket::write(const void* buf, size_t buf_size, size_t& out_size)
{
    DEBUG_ASSERT(buf && (buf_size > 0));

    out_size = 0;

    if (sock_ == bad_socket)
    {
        return error_code::invalid_socket;
    }

    if (!buf || (buf_size == 0))
    {
        return error_code::invalid_argument;
    }

    if (!is_writable())
    {
        return error_code::retry;
    }

    int rc = ::send(sock_, static_cast<const char*>(buf), static_cast<int>(buf_size), 0);

    if (rc > 0)
    {
        out_size = static_cast<size_t>(rc);
        return error_code::success;
    }

    int last_err = ::WSAGetLastError();

    switch (last_err)
    {
    case WSAEWOULDBLOCK:
        return error_code::retry;
    case WSAENOTCONN:
        return error_code::not_connected;
    case WSAECONNABORTED:
    case WSAECONNRESET:
        return error_code::connection_closed;
    }

    return error_code::unspecified;
}

} // namespace network
} // namespace fx

#endif
