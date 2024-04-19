#ifndef LCCL_INTERNAL_FILE_FILE_OS_H_
#define LCCL_INTERNAL_FILE_FILE_OS_H_

#include <string>
#include "lccl/file.h"

LCCL_BEGIN_NAMESPACE
LCCL_FILE_BEGIN_NAMESPACE

bool MkdirOs(const std::string &file_name);

void RmOs(const std::string &file_name, bool b_top_dir);

LCCL_FILE_END_NAMESPACE
LCCL_END_NAMESPACE

#endif // !LCCL_INTERNAL_FILE_FILE_OS_H_
