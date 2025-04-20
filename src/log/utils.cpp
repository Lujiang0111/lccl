#include <cassert>
#include "log/utils.h"

LCCL_BEGIN_NAMESPACE
LCCL_LOG_BEGIN_NAMESPACE

Utils::Utils() :
    next_id_(0)
{
    InitLevelMaps();
}

Utils::~Utils()
{

}

const Utils::LevelMap &Utils::GetLevelMap(Levels level)
{
    return level_mapping_.To(level);
}

size_t Utils::NextId()
{
    return next_id_.fetch_add(1);
}

void Utils::InitLevelMaps()
{
    level_mapping_.RegisterItem(Levels::kDebug, { fmt::terminal_color::bright_green, "D" });
    level_mapping_.RegisterItem(Levels::kInfo, { fmt::terminal_color::bright_blue, "I" });
    level_mapping_.RegisterItem(Levels::kWarn, { fmt::terminal_color::bright_yellow, "W" });
    level_mapping_.RegisterItem(Levels::kError, { fmt::terminal_color::bright_red, "E" });
}

LCCL_LOG_END_NAMESPACE
LCCL_END_NAMESPACE
