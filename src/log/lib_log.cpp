#include "log/lib_log.h"
#include "log/utils.h"

LCCL_BEGIN_NAMESPACE
LCCL_LOG_BEGIN_NAMESPACE

static void DefaultLogCallback(Levels level, const char *content, size_t len)
{
    fmt::println("[lccl]: {}{:.{}}", Utils::Instance().GetLvelMap(level).str, static_cast<int>(len), content);
}

static void (*lib_log_cb)(Levels level, const char *content, size_t len) = DefaultLogCallback;

void SetLibLogCallback(void (*cb)(Levels level, const char *content, size_t len))
{
    lib_log_cb = (cb) ? cb : DefaultLogCallback;
}

void LibLogContent(Levels level, const char *content, size_t len)
{
    lib_log_cb(level, content, len);
}

LCCL_LOG_END_NAMESPACE
LCCL_END_NAMESPACE
