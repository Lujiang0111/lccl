#ifndef LCCL_INCLUDE_LCCL_LOG_H_
#define LCCL_INCLUDE_LCCL_LOG_H_

#include <string>
#include "lccl.h"
#include "lccl/oss/fmt.h"

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
    virtual ~ILogger() = default;

    virtual void SetMaxLevel(Levels max_level) = 0;
    virtual void LogContent(Levels level, bool on_screen, bool sync, const char *file_name, int file_line, const char *content) = 0;

    template<typename... Args>
    inline void LogFmt(Levels level, bool on_screen, bool sync, const char *file_name, int file_line, fmt::format_string<Args...> fmt, Args &&... args)
    {
        std::string content = fmt::vformat(fmt, fmt::make_format_args(args...));
        LogContent(level, on_screen, sync, file_name, file_line, content.c_str());
    }
};

LCCL_API std::shared_ptr<ILogger> CreateLogger(const char *path, size_t max_size, CompressTypes compress_type);

LCCL_API ILogger *DefaultLogger();

LCCL_API void SetLcclLogCallback(void (*cb)(Levels level, const char *content, size_t len));

LCCL_LOG_END_NAMESPACE
LCCL_END_NAMESPACE

#define LCCL_DEFAULT_LOG_SYNC(level, fmt, ...) \
    lccl::log::DefaultLogger()->LogFmt(level, true, true, __FILE__, __LINE__, fmt, ##__VA_ARGS__);

#define LCCL_DEFAULT_LOG_ASYNC(level, fmt, ...) \
    lccl::log::DefaultLogger()->LogFmt(level, true, false, __FILE__, __LINE__, fmt, ##__VA_ARGS__);

#endif // !LCCL_INCLUDE_LCCL_LOG_H_
