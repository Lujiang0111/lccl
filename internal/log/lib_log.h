#ifndef LCCL_INTERNAL_LOG_LIB_LOG_H_
#define LCCL_INTERNAL_LOG_LIB_LOG_H_

#include <string>
#include <vector>
#include "lccl/fmt.h"
#include "lccl/log.h"

LCCL_BEGIN_NAMESPACE
LCCL_LOG_BEGIN_NAMESPACE

void LogContent(Levels level, const char *content, size_t len);

template<typename... Args>
inline void LogFmt(Levels level, fmt::format_string<Args...> fmt, Args &&... args)
{
    std::string content = fmt::vformat(fmt, fmt::make_format_args(args...));
    LogContent(level, content.c_str(), content.length());
}

LCCL_LOG_END_NAMESPACE
LCCL_END_NAMESPACE

#define LIB_LOG(level, fmt, ...) lccl::log::LogFmt(level, fmt, ##__VA_ARGS__);

#endif // !LCCL_INTERNAL_LOG_LIB_LOG_H_
