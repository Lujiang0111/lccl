#ifndef LCCL_INCLUDE_LCCL_LOG_H_
#define LCCL_INCLUDE_LCCL_LOG_H_

#include <cstdarg>
#include <cstddef>
#include <memory>
#include "lccl.h"

#define LCCL_LOG_BEGIN_NAMESPACE namespace log {
#define LCCL_LOG_END_NAMESPACE }

LCCL_BEGIN_NAMESPACE
LCCL_LOG_BEGIN_NAMESPACE

enum class Levels
{
    kDebug = 0,
    kInfo,
    kWarn,
    kError,
    kNb,
};

enum class CompressTypes
{
    kNone = 0,
    kGzip,
    kNb
};

class ILogger
{
public:
    ~ILogger() = default;

    virtual void Log(Levels level, bool on_screen, const char *file_name, int file_line, const char *content) = 0;
    virtual void LogPrintf(Levels level, bool on_screen, const char *file_name, int file_line, const char *fmt, ...) = 0;
};

LCCL_API std::shared_ptr<ILogger> CreateLogger(const char *path, size_t max_size, bool sync, CompressTypes compress_type);

LCCL_API void SetLibLogCallback(void (*cb)(Levels level, const char *content, size_t len));

LCCL_LOG_END_NAMESPACE
LCCL_END_NAMESPACE

#endif // !LCCL_INCLUDE_LCCL_LOG_H_
