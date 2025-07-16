#ifndef LCCL_INTERNAL_FILE_FILE_OS_H_
#define LCCL_INTERNAL_FILE_FILE_OS_H_

#include <string>
#include "lccl/file.h"

LCCL_NAMESPACE_BEGIN
LCCL_FILE_NAMESPACE_BEGIN

bool MkdirOs(const std::string &file_name);

void RmOs(const std::string &file_name, bool b_top_dir);

size_t GetPathFreeSpaceOs(const std::string &path);

LCCL_FILE_NAMESPACE_END
LCCL_NAMESPACE_END

#endif // !LCCL_INTERNAL_FILE_FILE_OS_H_
