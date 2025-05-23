﻿#ifndef LCCL_INCLUDE_LCCL_OSS_YAML_H_
#define LCCL_INCLUDE_LCCL_OSS_YAML_H_

#include <string>
#include "lccl/oss/rapidyaml/rapidyaml.hpp"

inline std::string C4ToString(const c4::csubstr &str)
{
    return std::string(str.str, str.len);
}

#endif // !LCCL_INCLUDE_LCCL_OSS_YAML_H_
