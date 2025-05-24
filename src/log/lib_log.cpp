#include "log/lib_log.h"
#include "log/utils.h"

LCCL_BEGIN_NAMESPACE
LCCL_LOG_BEGIN_NAMESPACE

static void DefaultLogCallback(void *opaque, Levels level, const char *file_name, int file_line, const char *content, size_t len)
{
    fmt::println("[pcap_dump]: {} {}:{} {:.{}}",
        Utils::Instance()->GetLevelMap(level).str,
        file_name, file_line,
        content, len);
}

static void (*lib_log_cb)(void *opaque, Levels level, const char *file_name, int file_line, const char *content, size_t len) = DefaultLogCallback;
static void *lib_log_opaque = nullptr;

void SetLcclLogCallback(
    void (*cb)(void *opaque, Levels level, const char *file_name, int file_line, const char *content, size_t len),
    void *opaque)
{
    lib_log_cb = (cb) ? cb : DefaultLogCallback;
    lib_log_opaque = opaque;
}

void LibLogContent(Levels level, const char *file_name, int file_line, const char *content, size_t len)
{
    lib_log_cb(lib_log_opaque, level, file_name, file_line, content, len);
}

LCCL_LOG_END_NAMESPACE
LCCL_END_NAMESPACE
