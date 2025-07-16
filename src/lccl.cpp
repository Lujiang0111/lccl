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

LCCL_NAMESPACE_END
