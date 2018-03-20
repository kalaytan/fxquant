#if defined(_WIN32)
#include <winsock2.h>
#pragma warning(suppress: 6031)

namespace { // anonymous namespace

struct winsock_init
{
    winsock_init()
    {
        WSADATA data;
        WORD ver = MAKEWORD(2, 2);
        ::WSAStartup(ver, &data);
    }

    ~winsock_init()
    {
        ::WSACleanup();
    }
};

} // anonymous namespace

extern "C"
{
    // global object
    winsock_init winsock_initializer;
}
#endif // _WIN32
