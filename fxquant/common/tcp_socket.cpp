#include <chrono>

#if !defined(_WIN32)
#include <netinet/tcp.h>
#endif

#include "debug.h"
#include "socket.h"

using namespace std::chrono;

namespace fx {
namespace network {

typedef socket::error_code error_code;

tcp_socket::~tcp_socket()
{
    tcp_socket::close();
}

void tcp_socket::close()
{
    if (sock_ != bad_socket)
    {
        if (connected_)
        {
            ::shutdown(sock_, 1); // SD_SEND
            
            if (wait_for_readable(100ms) == error_code::success)
            {
                char buf[256] = { 0 };

                for ( ; ; )
                {
                    size_t bytes = 0;
                    error_code err = read(buf, sizeof(buf), bytes);

                    if ((err != error_code::success) || (bytes == 0))
                    {
                        break;
                    }
                }
            }
        }

        socket::close();
        DEBUG_ENSURE(sock_ == bad_socket);
    }
}

error_code tcp_socket::set_no_delay(bool on_off)
{
    int opt = on_off ? 1 : 0;
    return set_option(TCP_NODELAY, IPPROTO_TCP, opt);
}

error_code tcp_socket::set_keep_alive(bool on_off)
{
    int opt = on_off ? 1 : 0;
    return set_option(SO_KEEPALIVE, IPPROTO_TCP, opt);
}

error_code tcp_socket::listen()
{
    if (sock_ == bad_socket)
    {
        return error_code::invalid_socket;
    }

    int rc = ::listen(sock_, SOMAXCONN);

    return (rc == 0) ? error_code::success : error_code::unspecified;
}

tcp_socket_ptr tcp_socket::accept()
{
    tcp_socket_ptr sock_ptr;

    if (sock_ == bad_socket)
    {
        return sock_ptr; // zero
    }

    socket_type sock = ::accept(sock_, NULL, NULL);

    if (sock != bad_socket)
    {
        sock_ptr = tcp_socket_ptr(new tcp_socket(sock));
    }

    return sock_ptr;
}

error_code tcp_socket::peek(void* buf, size_t buf_size, size_t& out_size)
{
    return read(buf, buf_size, out_size, true);
}

error_code tcp_socket::read(void* buf, size_t buf_size, size_t& out_size)
{
    return read(buf, buf_size, out_size, false);
}

error_code tcp_socket::read(void* buf, size_t buf_size, size_t& out_size, timeout_type timeout)
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

    if (timeout.count() == 0) // zero timeout
    {
        error_code err = read(buf, buf_size, out_size);

        if (err != error_code::success)
        {
            return err; // read() failed
        }
    }
    else // finite timeout
    {
        auto time_left = timeout;
        auto time_start = system_clock::now();

        for ( ; ; )
        {
            error_code err = wait_for_readable(time_left);

            if (err != error_code::success)
            {
                break; // timeout expired
            }

            size_t size = 0;
            size_t bytes_left = buf_size - out_size;
            err = read(static_cast<uint8_t*>(buf) + out_size, bytes_left, size);

            if (err != error_code::success)
            {
                return err; // read() failed
            }

            out_size += size;

            if (out_size >= buf_size)
            {
                break; // done
            }

            auto time_now = system_clock::now();
            auto time_spent = time_now - time_start;

            if (time_spent >= timeout)
            {
                break; // timeout expired
            }

            time_left = duration_cast<milliseconds>(timeout - time_spent);
        }
    }

    return (out_size < buf_size) ? error_code::timeout : error_code::success;
}

error_code tcp_socket::write(const void* buf, size_t buf_size, size_t& out_size, timeout_type timeout)
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

    if (timeout.count() == 0) // zero timeout
    {
        error_code err = write(buf, buf_size, out_size);

        if (err != error_code::success)
        {
            return err; // write() failed
        }
    }
    else // finite timeout
    {
        auto time_left = timeout;
        auto time_start = system_clock::now();

        for ( ; ; )
        {
            error_code err = wait_for_writable(time_left);

            if (err != error_code::success)
            {
                break; // timeout expired
            }

            size_t size = 0;
            size_t bytes_left = buf_size - out_size;
            err = write(static_cast<const uint8_t*>(buf) + out_size, bytes_left, size);

            if (err != error_code::success)
            {
                return err; // write() failed
            }

            out_size += size;

            if (out_size >= buf_size)
            {
                break; // done
            }

            auto time_now = system_clock::now();
            auto time_spent = time_now - time_start;

            if (time_spent >= timeout)
            {
                break; // timeout expired
            }

            time_left = duration_cast<milliseconds>(timeout - time_spent);
        }
    }

    return (out_size < buf_size) ? error_code::timeout : error_code::success;
}

} // namespace network
} // namespace fx
