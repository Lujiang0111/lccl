#ifndef LCCL_INCLUDE_UTILS_FILE_INL_H_
#define LCCL_INCLUDE_UTILS_FILE_INL_H_

#include <algorithm>
#include <string>
#include "lccl.h"

LCCL_BEGIN_NAMESPACE

constexpr char kDirSeps[] = { '\\', '/' };

#if defined(_WIN32)
constexpr char kDirSep = kDirSeps[0];
#else
constexpr char kDirSep = kDirSeps[1];
#endif

inline std::string AdjustOsPath(const std::string &path)
{
    std::string result;
    bool b_sep = false;
    bool b_prev_sep = false;
    for (char c : path)
    {
        b_sep = false;
        for (auto &&sep : kDirSeps)
        {
            if (sep == c)
            {
                b_sep = true;
                break;
            }
        }

        if (b_sep)
        {
            if (!b_prev_sep)
            {
                result.push_back(kDirSep);
            }
        }
        else
        {
            result.push_back(c);
        }
        b_prev_sep = b_sep;
    }

    if ((result.length() > 0) && ('/' == result.back()))
    {
        result.pop_back();
    }
    return result;
}

inline std::string OsPathJoin(const std::string &path)
{
    return AdjustOsPath(path);
}

template<typename... Args>
inline std::string OsPathJoin(const std::string &first, const Args&... args)
{
    return AdjustOsPath(first) + std::to_string(kDirSep) + OsPathJoin(args...);
}

LCCL_END_NAMESPACE

#endif // !LCCL_INCLUDE_UTILS_FILE_INL_H_
