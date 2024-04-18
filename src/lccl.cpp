#include "lccl.h"

#ifndef VERSION_X
#define VERSION_X 1
#endif

#ifndef VERSION_Y
#define VERSION_Y 0
#endif

#ifndef VERSION_Z
#define VERSION_Z 0
#endif

#define STRINGIFY(x) #x
#define GET_VERSION(x, y, z) STRINGIFY(x) "." STRINGIFY(y) "." STRINGIFY(z)

LCCL_BEGIN_NAMESPACE

const char *GetVersion()
{
    const char *version = GET_VERSION(VERSION_X, VERSION_Y, VERSION_Z);
    return version;
}

LCCL_END_NAMESPACE
