#include "lccl/utils/path.h"
#include "file/file_info.h"
#include "log/lib_log.h"

LCCL_BEGIN_NAMESPACE
LCCL_FILE_BEGIN_NAMESPACE

FileInfo::FileInfo() :
    total_size_(0),
    modify_timestamp_(0),
    file_mode_(FileModes::kUnspecified)
{

}

FileInfo::~FileInfo()
{
    childs_.clear();
}

const char *FileInfo::GetName() const
{
    return name_.c_str();
}

const char *FileInfo::GetRelativeName() const
{
    return relative_name_.c_str();
}

FileModes FileInfo::GetFileMode() const
{
    return file_mode_;
}

size_t FileInfo::GetTotalSize() const
{
    return total_size_;
}

int64_t FileInfo::GetModifyTimestamp() const
{
    return modify_timestamp_;
}

size_t FileInfo::GetChildCnt() const
{
    return childs_.size();
}

const IFileInfo *FileInfo::GetChild(size_t idx) const
{
    if (idx >= childs_.size())
    {
        return nullptr;
    }

    return childs_[idx].get();
}

std::shared_ptr<FileInfo> FileInfo::CreateFileInfo(const std::string &file_name, SortTypes sort_type)
{
    std::string os_file_name = OsPathJoin(file_name);
    std::shared_ptr<FileInfo> file_info = FindFileInfo(os_file_name);
    if (!file_info)
    {
        return nullptr;
    }

    SortFileInfo(file_info.get(), sort_type);
    return file_info;
}

void FileInfo::SortFileInfo(FileInfo *file_info, SortTypes sort_type)
{
    if ((!file_info) || (SortTypes::kNone == sort_type))
    {
        return;
    }

    // sort subdir first
    for (auto &&child : file_info->childs_)
    {
        SortFileInfo(child.get(), sort_type);
    }

    // sort
    std::sort(file_info->childs_.begin(), file_info->childs_.end(),
        [sort_type](const std::shared_ptr<FileInfo> &lhs, const std::shared_ptr<FileInfo> &rhs) {
            switch (sort_type)
            {
            case SortTypes::kByName:
                return (lhs->name_.compare(rhs->name_) < 0);
            case SortTypes::kByModifyTime:
                return (lhs->modify_timestamp_ < rhs->modify_timestamp_);
            default:
                break;
            }
            return true;
        });
}

std::shared_ptr<IFileInfo> CreateFileInfo(const char *file_name, SortTypes sort_type)
{
    if (!file_name)
    {
        LIB_LOG(log::Levels::kError, "GetFileInfo : file name not exist!");
        return nullptr;
    }

    return FileInfo::CreateFileInfo(file_name, sort_type);
}


LCCL_FILE_END_NAMESPACE
LCCL_END_NAMESPACE
