﻿#ifndef LCCL_INTERNAL_LOG_LIB_LOG_H_
#define LCCL_INTERNAL_LOG_LIB_LOG_H_

#include "lccl/log.h"

LCCL_BEGIN_NAMESPACE
LCCL_LOG_BEGIN_NAMESPACE

void LibLogContent(Levels level, const char *content, size_t len);

template<typename... Args>
inline void LibLogFmt(Levels level, fmt::format_string<Args...> fmt, Args &&... args)
{
    std::string content = fmt::vformat(fmt, fmt::make_format_args(args...));
    LibLogContent(level, content.c_str(), content.length());
}

LCCL_LOG_END_NAMESPACE
LCCL_END_NAMESPACE

#define LIB_LOG(level, fmt, ...) lccl::log::LibLogFmt(level, fmt, ##__VA_ARGS__);

#endif // !LCCL_INTERNAL_LOG_LIB_LOG_H_
