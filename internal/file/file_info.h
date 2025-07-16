#ifndef LCCL_INTERNAL_FILE_FILE_INFO_H_
#define LCCL_INTERNAL_FILE_FILE_INFO_H_

#include <string>
#include <vector>
#include "lccl/file.h"

LCCL_NAMESPACE_BEGIN
LCCL_FILE_NAMESPACE_BEGIN

class FileInfo : public IFileInfo
{
public:
    FileInfo(const FileInfo &) = delete;
    FileInfo &operator=(const FileInfo &) = delete;

    FileInfo();
    virtual ~FileInfo();

    virtual const char *GetName() const;
    virtual const char *GetRelativeName() const;
    virtual FileModes GetFileMode() const;
    virtual size_t GetTotalSize() const;
    virtual int64_t GetModifyTimestamp() const;

    virtual size_t GetChildCnt() const;
    virtual const IFileInfo *GetChild(size_t idx) const;

    static std::shared_ptr<FileInfo> CreateFileInfo(const std::string &file_name, SortTypes sort_type);

private:
    static std::shared_ptr<FileInfo> FindFileInfo(const std::string &file_name);
    static void SortFileInfo(FileInfo *file_info, SortTypes sort_type);

#if defined(_MSC_VER)
    static void FindChildFileInfo(FileInfo *parent);
#endif

private:
    std::string name_;
    std::string relative_name_;
    size_t total_size_;
    int64_t modify_timestamp_;
    FileModes file_mode_;

    std::vector<std::shared_ptr<FileInfo>> childs_;
};

LCCL_FILE_NAMESPACE_END
LCCL_NAMESPACE_END

#endif // !LCCL_INTERNAL_FILE_FILE_INFO_H_
