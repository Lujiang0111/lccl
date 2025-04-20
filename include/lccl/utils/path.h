#ifndef LCCL_INCLUDE_LCCL_UTILS_PATH_H_
#define LCCL_INCLUDE_LCCL_UTILS_PATH_H_

#include <algorithm>
#include <string>
#include "lccl.h"

LCCL_BEGIN_NAMESPACE

constexpr char kDirSeps[] = { '\\', '/' };

#if defined(_MSC_VER)
constexpr char kDirSep = kDirSeps[0];
#else
constexpr char kDirSep = kDirSeps[1];
#endif

inline std::string AdjustPath(const std::string &path, char sep_char = kDirSep)
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
                result.push_back(sep_char);
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
    return AdjustPath(path);
}

template<typename... Args>
inline std::string OsPathJoin(const std::string &first, const Args&... args)
{
    return AdjustPath(first) + kDirSep + OsPathJoin(args...);
}

LCCL_END_NAMESPACE

#endif // !LCCL_INCLUDE_LCCL_UTILS_PATH_H_
