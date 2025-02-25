#include <cassert>
#include "log/utils.h"

LCCL_BEGIN_NAMESPACE
LCCL_LOG_BEGIN_NAMESPACE



Utils &Utils::Instance()
{
    static Utils instance;
    return instance;
}

const Utils::LevelMap &Utils::GetLvelMap(Levels level) const
{
    return level_maps_[static_cast<size_t>(level)];
}

Utils::Utils()
{
    InitLevelMaps();
}

void Utils::InitLevelMaps()
{
    level_maps_.resize(static_cast<size_t>(Levels::kNb));
    level_maps_[static_cast<size_t>(Levels::kDebug)] = { fmt::terminal_color::bright_green, "D" };
    level_maps_[static_cast<size_t>(Levels::kInfo)] = { fmt::terminal_color::bright_blue, "I" };
    level_maps_[static_cast<size_t>(Levels::kWarn)] = { fmt::terminal_color::bright_yellow, "W" };
    level_maps_[static_cast<size_t>(Levels::kError)] = { fmt::terminal_color::bright_red, "E" };
}

LCCL_LOG_END_NAMESPACE
LCCL_END_NAMESPACE
