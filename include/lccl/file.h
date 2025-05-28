#ifndef LCCL_INCLUDE_LCCL_FILE_H_
#define LCCL_INCLUDE_LCCL_FILE_H_

#include "lccl.h"

#define LCCL_FILE_BEGIN_NAMESPACE namespace file {
#define LCCL_FILE_END_NAMESPACE }

LCCL_BEGIN_NAMESPACE
LCCL_FILE_BEGIN_NAMESPACE

enum class FileModes
{
    kUnspecified = 0,
    kRegularFile,
    kDirectory,
};

enum class SortTypes
{
    kNone = 0,
    kByName,
    kByModifyTime,
};

class IFileInfo
{
public:
    virtual ~IFileInfo() = default;

    virtual const char *GetName() const = 0;
    virtual const char *GetRelativeName() const = 0;
    virtual FileModes GetFileMode() const = 0;
    virtual size_t GetTotalSize() const = 0;
    virtual int64_t GetModifyTimestamp() const = 0;

    virtual size_t GetChildCnt() const = 0;
    virtual const IFileInfo *GetChild(size_t idx) const = 0;
};

LCCL_API std::shared_ptr<IFileInfo> CreateFileInfo(const char *file_name, SortTypes sort_type);

class IFileMmap
{
public:
    virtual ~IFileMmap() = default;

    virtual const uint8_t *GetData() const = 0;
    virtual size_t GetSize() const = 0;
};

LCCL_API std::shared_ptr<IFileMmap> CreateFileMmap(const char *file_name);

LCCL_API bool CreateDir(const char *file_name, bool parent_only);

LCCL_API void RemoveFile(const char *file_name);

LCCL_API size_t GetPathFreeSpace(const char *path);

LCCL_FILE_END_NAMESPACE
LCCL_END_NAMESPACE

#endif // !LCCL_INCLUDE_LCCL_FILE_H_
