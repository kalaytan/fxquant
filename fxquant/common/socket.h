#pragma once

#if defined(_MSC_VER) // force compiler to instantiate the winsock_initializer object
#if defined(_WIN64)
#pragma comment(linker, "/include:winsock_initializer")
#else
#pragma comment(linker, "/include:_winsock_initializer")
#endif
#pragma comment(lib, "ws2_32.lib")
#endif // _MSC_VER

#include <chrono>
#include <memory>
#include <vector>
#include "types.h"
#include "inet_address.h"

namespace fx {
namespace network {

class socket // a base class for all socket types
{
public:
    enum class error_code
    {
        success            = 0,
        invalid_argument   = 1,
        invalid_socket     = 2,
        timeout            = 3,
        too_many_sockets   = 4,
        already_bound      = 5,
        already_connected  = 6,
        canceled           = 7,
        network_unreached  = 8,
        host_unreached     = 9,
        connection_closed  = 10,
        connection_refused = 11,
        no_free_ports      = 12,
        retry              = 13,
        no_data            = 14,
        not_connected      = 15,
        buffer_too_small   = 16,
        unspecified        = -1
    };

#if defined(_WIN32)
    typedef int socklen_type;
    typedef SOCKET socket_type;
    static const socket_type bad_socket = INVALID_SOCKET;
#else
    typedef socklen_t socklen_type;
    typedef int socket_type;
    static const socket_type bad_socket = -1;
#endif

protected:
    socket() : sock_(bad_socket) {}
    explicit socket(socket_type sock) : sock_(sock) {}

public:
    virtual ~socket();

    error_code get_recv_bufsize(size_t& size) const;
    error_code set_recv_bufsize(size_t size);

    error_code get_send_bufsize(size_t& size) const;
    error_code set_send_bufsize(size_t size);

    error_code set_reuse_address(bool on_off = true);
    error_code set_blocking_mode(bool on_off = true);

    bool is_readable() const;
    error_code wait_for_readable() const; // infinite wait
    error_code wait_for_readable(timeout_type timeout) const;
    
    bool is_writable() const;
    error_code wait_for_writable() const; // infinite wait
    error_code wait_for_writable(timeout_type timeout) const;
    
    inet_address_ptr get_socket_address() const;
    inet_address_ptr get_peer_address() const;

    virtual void close();

    socket_type get_native_socket() const
    {
        return sock_;
    }

protected:
    int get_socket_error();
    error_code bind(const inet_address& addr);

    template <typename T>
    error_code set_option(int code, int level, const T& value)
    {
        if (sock_ == bad_socket)
        {
            return error_code::invalid_socket;
        }

        int rc = setsockopt(sock_, level, code,
            reinterpret_cast<const char*>(&value), sizeof(T));

        return (rc == 0) ? error_code::success : error_code::unspecified;
    }

    template <typename T>
    error_code get_option(int code, int level, T& value) const
    {
        if (sock_ == bad_socket)
        {
            return error_code::invalid_socket;
        }

        socklen_type size = sizeof(T);
        int rc = getsockopt(sock_, level, code, reinterpret_cast<char*>(&value), &size);
        return (rc == 0) ? error_code::success : error_code::unspecified;
    }

protected:
    mutable socket_type sock_;
};

class tcp_socket : public socket // a base class for TCP sockets
{
public:
    typedef std::shared_ptr<tcp_socket> tcp_socket_ptr;

public:
    tcp_socket() : connected_(false) {}
    ~tcp_socket();

private:
    // used internally by accept()
    explicit tcp_socket(socket_type sock) : connected_(true)
    {
        sock_ = sock;
    }

public:
    error_code listen();
    tcp_socket_ptr accept();

    error_code peek(void* buf, size_t buf_size, size_t& out_size); // non-blocking call

    error_code read(void* buf, size_t buf_size, size_t& out_size); // non-blocking call
    error_code read(void* buf, size_t buf_size, size_t& out_size, timeout_type timeout);

    error_code write(const void* buf, size_t buf_size, size_t& out_size); // non-blocking call
    error_code write(const void* buf, size_t buf_size, size_t& out_size, timeout_type timeout);

    error_code set_no_delay(bool on_off = true);
    error_code set_keep_alive(bool on_off = true);

    void close();

protected:
    error_code connect(const inet_address& addr, timeout_type timeout);

private:
    error_code read(void* buf, size_t buf_size, size_t& out_size, bool peek); // non-blocking call

private:
    bool connected_;
};

template <typename T, int family>
class tcpx_socket : public tcp_socket
{
public:
    tcpx_socket()
    {
        sock_ = ::socket(family, SOCK_STREAM, 0);
    }

    error_code bind(const T& addr)
    {
        return socket::bind(addr);
    }

    error_code connect(const T& addr, timeout_type timeout)
    {
        return tcp_socket::connect(addr, timeout);
    }

    error_code connect(const T& addr) // infinite timeout
    {
        return tcp_socket::connect(addr);
    }
};

typedef tcpx_socket<inet4_address, AF_INET> tcp4_socket;
typedef tcpx_socket<inet6_address, AF_INET6> tcp6_socket;

typedef std::shared_ptr<socket> socket_ptr;
typedef std::shared_ptr<tcp_socket> tcp_socket_ptr;

} // namespace network
} // namespace fx
