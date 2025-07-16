#ifndef LCCL_INCLUDE_LCCL_UTILS_STRING_UTILS_H_
#define LCCL_INCLUDE_LCCL_UTILS_STRING_UTILS_H_

#include <algorithm>
#include <cctype>
#include <string>
#include "lccl.h"

LCCL_NAMESPACE_BEGIN

inline std::string TrimString(const std::string &str)
{
    auto start = std::find_if(str.begin(), str.end(), [](unsigned char ch) {
        return !std::isspace(ch);
        });
    auto end = std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
        }).base();

    return (start < end) ? std::string(start, end) : std::string();
}

LCCL_NAMESPACE_END

#endif // !LCCL_INCLUDE_LCCL_UTILS_STRING_UTILS_H_
