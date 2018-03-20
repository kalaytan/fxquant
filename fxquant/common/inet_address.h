#if defined(_WIN32)
#include <winsock2.h>
#include <ws2ipdef.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include <memory>
#include <string>

namespace fx {
namespace network {

typedef uint16_t port_type;
enum class address_type { any = 0, loopback = 1, broadcast = 2 };

class inet_address // a base class
{
public:
    virtual port_type get_port() const = 0;
    virtual void set_port(port_type port) = 0;
    
    virtual bool from_string(const std::string& addr) = 0;
    virtual std::string to_string() const = 0;

    virtual operator const sockaddr*() const = 0;
    virtual size_t size() const = 0;
};

class inet4_address : public inet_address
{
public:
    explicit inet4_address(port_type port = 0);
    explicit inet4_address(const std::string& addr, port_type port = 0);
    explicit inet4_address(address_type atype, port_type port = 0);
    explicit inet4_address(const sockaddr* sa);

    void set_port(port_type port) override
    {
        addr_.sin_port = htons(port);
    }

    port_type get_port() const override
    {
        return ntohs(addr_.sin_port);
    }

    operator const sockaddr*() const override
    {
        return reinterpret_cast<const sockaddr*>(&addr_);
    }

    size_t size() const override
    {
        return sizeof(addr_);
    }

    bool from_string(const std::string& addr) override;
    std::string to_string() const override;

    inet4_address& operator =(const inet4_address& rhs);
    bool operator ==(const inet4_address& rhs) const;

private:
    sockaddr_in addr_;
};

class inet6_address : public inet_address
{
public:
    explicit inet6_address(port_type port = 0);
    explicit inet6_address(const std::string& addr, port_type port = 0);
    explicit inet6_address(address_type atype, port_type port = 0);
    explicit inet6_address(const sockaddr* sa);

    port_type get_port() const override
    {
        return ntohs(addr_.sin6_port);
    }

    void set_port(port_type port) override
    {
        addr_.sin6_port = htons(port);
    }

    operator const sockaddr*() const override
    {
        return reinterpret_cast<const sockaddr*>(&addr_);
    }

    size_t size() const override
    {
        return sizeof(addr_);
    }

    bool from_string(const std::string& addr) override;
    std::string to_string() const override;

    inet6_address& operator =(const inet6_address& rhs);
    bool operator ==(const inet6_address& rhs) const;

private:
    sockaddr_in6 addr_;
};

typedef std::shared_ptr<inet_address> inet_address_ptr;

} // namespace network
} // namespace fx
