﻿#ifndef LCCL_INTERNAL_LOG_UTILS_H_
#define LCCL_INTERNAL_LOG_UTILS_H_

#include <vector>
#include "lccl/log.h"

LCCL_BEGIN_NAMESPACE
LCCL_LOG_BEGIN_NAMESPACE

class Utils
{
public:
    struct LevelMap
    {
        fmt::terminal_color fg_color;
        const char *str;
    };

public:
    virtual ~Utils() = default;
    static Utils &Instance();

    const LevelMap &GetLvelMap(Levels level) const;

private:
    Utils();

    void InitLevelMaps();

private:
    std::vector<LevelMap> level_maps_;
};

LCCL_LOG_END_NAMESPACE
LCCL_END_NAMESPACE

#endif // !LCCL_INTERNAL_LOG_UTILS_H_
