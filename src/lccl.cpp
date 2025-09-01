#if defined(_MSC_VER)
#include <ws2tcpip.h>
#else
#endif

#include "lccl.h"

#ifndef VERSION_X
#define VERSION_X 1
#endif

#ifndef VERSION_Y
#define VERSION_Y 0
#endif

#define STRINGIFY(x) #x
#define GET_VERSION(x, y) STRINGIFY(x) "." STRINGIFY(y)

LCCL_NAMESPACE_BEGIN

const char *GetVersion()
{
    const char *version = GET_VERSION(VERSION_X, VERSION_Y);
    return version;
}

bool InitEnv()
{
#if defined(_MSC_VER)
    timeBeginPeriod(1);

    WORD wsa_version = MAKEWORD(2, 2);
    WSADATA wsa_data;
    if (0 != WSAStartup(wsa_version, &wsa_data))
    {
        return false;
    }

    if ((2 != LOBYTE(wsa_data.wVersion)) || (2 != HIBYTE(wsa_data.wVersion)))
    {
        WSACleanup();
        return false;
    }
#else
#endif

    return true;
}

void DeinitEnv()
{
#if defined(_MSC_VER)
    WSACleanup();
    timeEndPeriod(1);
#else
#endif
}

LCCL_NAMESPACE_END
