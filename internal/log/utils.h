#ifndef LCCL_INTERNAL_LOG_UTILS_H_
#define LCCL_INTERNAL_LOG_UTILS_H_

#include <atomic>
#include <vector>
#include "lccl/log.h"
#include "lccl/utils/enum_utils.h"

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
    inline static Utils *Instance()
    {
        static Utils instance;
        return &instance;
    }

    virtual ~Utils();

    const LevelMap &GetLevelMap(Levels level);
    size_t NextId();

private:
    Utils();

    void InitLevelMaps();

private:
    EnumMapping<Levels, LevelMap> level_mapping_;
    std::atomic<size_t> next_id_;
};

LCCL_LOG_END_NAMESPACE
LCCL_END_NAMESPACE

#endif // !LCCL_INTERNAL_LOG_UTILS_H_
