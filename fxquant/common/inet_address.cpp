#if defined(_WIN32)
#include <ws2tcpip.h>
#else
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include <cstring> // memset
#include <stdexcept>
#include "inet_address.h"

namespace fx {
namespace network {

//-----------------------------------------------------------------------------
// inet4_address
//-----------------------------------------------------------------------------

inet4_address::inet4_address(port_type port)
{
    std::memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = INADDR_ANY;
    set_port(port);
}

inet4_address::inet4_address(const std::string& addr, port_type port)
{
    std::memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;

    if (!from_string(addr))
    {
        throw std::invalid_argument("inet4_address::inet4_address()");
    }

    set_port(port);
}

inet4_address::inet4_address(address_type atype, port_type port)
{
    std::memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    set_port(port);

    switch (atype)
    {
    case address_type::loopback:
        addr_.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        break;
    case address_type::any:
        addr_.sin_addr.s_addr = htonl(INADDR_ANY);
        break;
    case address_type::broadcast:
        addr_.sin_addr.s_addr = htonl(INADDR_BROADCAST);
        break;
    }
}

inet4_address::inet4_address(const sockaddr* sa)
{
    if (!sa || (sa->sa_family != AF_INET))
    {
        throw std::invalid_argument("inet4_address::inet4_address()");
    }

    std::memcpy(&addr_, sa, sizeof(addr_));
}

bool inet4_address::from_string(const std::string& saddr)
{
    if (!saddr.empty())
    {
        struct addrinfo* res = NULL;
        struct addrinfo hints;
        
        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        int err = getaddrinfo(saddr.c_str(), NULL, &hints, &res);

        if ((err == 0) && (res != NULL) && (res->ai_family == AF_INET) &&
            (res->ai_addrlen == sizeof(addr_)))
        {
            std::memcpy(&addr_, res->ai_addr, res->ai_addrlen);
            return true;
        }
    }

    return false;
}

std::string inet4_address::to_string() const
{
    return inet_ntoa(addr_.sin_addr);
}

inet4_address& inet4_address::operator = (const inet4_address& rhs)
{
    if (this != &rhs)
    {
        std::memcpy(&addr_, &rhs.addr_, sizeof(addr_));
    }

    return *this;
}

bool inet4_address::operator ==(const inet4_address& rhs) const
{
    if ((addr_.sin_family == rhs.addr_.sin_family) &&
        (addr_.sin_port == rhs.addr_.sin_port))
    {
        return (std::memcmp(&addr_.sin_addr, &rhs.addr_.sin_addr,
            sizeof(addr_.sin_addr)) == 0);
    }

    return false;
}

//-----------------------------------------------------------------------------
// inet6_address
//-----------------------------------------------------------------------------

inet6_address::inet6_address(port_type port)
{
    std::memset(&addr_, 0, sizeof(addr_));
    addr_.sin6_family = AF_INET6;
    addr_.sin6_addr = in6addr_any;
    set_port(port);
}

inet6_address::inet6_address(const std::string& addr, port_type port)
{
    std::memset(&addr_, 0, sizeof(addr_));
    addr_.sin6_family = AF_INET6;

    if (!from_string(addr))
    {
        throw std::invalid_argument("inet6_address::inet6_address()");
    }

    set_port(port);
}

inet6_address::inet6_address(address_type atype, port_type port)
{
    std::memset(&addr_, 0, sizeof(addr_));
    addr_.sin6_family = AF_INET6;
    set_port(port);

    switch (atype)
    {
    case address_type::loopback :
        addr_.sin6_addr = in6addr_loopback;
        break;
    case address_type::any:
        addr_.sin6_addr = in6addr_any;
        break;
    default:
        throw std::invalid_argument("inet6_address::inet6_address()");
    }
}

inet6_address::inet6_address(const sockaddr* sa)
{
    if (!sa || (sa->sa_family != AF_INET6))
    {
        throw std::invalid_argument("inet6_address::inet6_address()");
    }

    std::memcpy(&addr_, sa, sizeof(addr_));
}

bool inet6_address::from_string(const std::string& saddr)
{
    if (!saddr.empty())
    {
        struct addrinfo* res = NULL;
        struct addrinfo hints;

        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET6;
        hints.ai_socktype = SOCK_STREAM;

        int err = getaddrinfo(saddr.c_str(), NULL, &hints, &res);

        if ((err == 0) && (res != NULL) && (res->ai_family == AF_INET6) &&
            (res->ai_addrlen == sizeof(addr_)))
        {
            std::memcpy(&addr_, res->ai_addr, res->ai_addrlen);
            return true;
        }
    }

    return false;
}

std::string inet6_address::to_string() const
{
    char saddr[INET6_ADDRSTRLEN] = { 0 };

#if defined(_WIN32)
    DWORD len = sizeof(saddr);
    
    sockaddr_in6 tmp;
    std::memcpy(&tmp, &addr_, sizeof(tmp));
    tmp.sin6_port = 0; // important!

    int err = WSAAddressToStringA((LPSOCKADDR)&tmp, sizeof(tmp),
        NULL, saddr, &len);

    if (err != 0)
    {
        saddr[0] = 0;
    }
#else
    const char* p = inet_ntop(AF_INET6,
        &addr_.sin6_addr, saddr, sizeof(saddr));

    if (p == NULL)
    {
        saddr[0] = 0;
    }
#endif
    
    return saddr;
}

inet6_address& inet6_address::operator =(const inet6_address& rhs)
{
    if (this != &rhs)
    {
        std::memcpy(&addr_, &rhs.addr_, sizeof(addr_));
    }

    return *this;
}

bool inet6_address::operator ==(const inet6_address& rhs) const
{
    if ((addr_.sin6_family == rhs.addr_.sin6_family) &&
        (addr_.sin6_port == rhs.addr_.sin6_port))
    {
        return (std::memcmp(&addr_.sin6_addr, &rhs.addr_.sin6_addr,
            sizeof(addr_.sin6_addr)) == 0);
    }
    
    return false;
}

} // namespace network
} // namespace fx
