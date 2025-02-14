#if defined(_WIN32)

#include <windows.h>
#include <direct.h>
#include "lccl/utils/path.h"
#include "file/file_info.h"
#include "log/lib_log.h"

LCCL_BEGIN_NAMESPACE
LCCL_FILE_BEGIN_NAMESPACE

std::shared_ptr<FileInfo> FileInfo::FindFileInfo(const std::string &file_name)
{
    WIN32_FIND_DATAA ffd;
    HANDLE file_handle = FindFirstFileA(file_name.c_str(), &ffd);
    if (INVALID_HANDLE_VALUE == file_handle)
    {
        return nullptr;
    }

    std::shared_ptr<FileInfo> file_info = std::make_shared<FileInfo>();
    file_info->name_ = ffd.cFileName;
    file_info->relative_name_ = file_name;
    file_info->total_size_ = (static_cast<size_t>(ffd.nFileSizeHigh) << 32) | static_cast<size_t>(ffd.nFileSizeLow);

    uint64_t total_us = ((static_cast<uint64_t>(ffd.ftLastWriteTime.dwHighDateTime) << 32) |
        (static_cast<uint64_t>(ffd.ftLastWriteTime.dwLowDateTime))) / 10 - 11644473600000000ull;
    file_info->modify_timestamp_ = static_cast<int64_t>(total_us / 1000000);

    if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        file_info->file_mode_ = FileModes::kDirectory;
        FindChildFileInfo(file_info.get());
    }
    else
    {
        file_info->file_mode_ = FileModes::kRegularFile;
    }
    return file_info;
}

void FileInfo::FindChildFileInfo(FileInfo *parent)
{
    /*
    Prepare string for use with FindFile functions.First, copy the
    string to a buffer, then append '\*' to the directory name.
    */
    std::string search_name = parent->relative_name_ + "\\*";

    // Find the first file in the directory.
    WIN32_FIND_DATAA ffd;
    HANDLE file_handle = FindFirstFileA(search_name.c_str(), &ffd);
    if (INVALID_HANDLE_VALUE == file_handle)
    {
        return;
    }

    do
    {
        // Find first file will always return "." and ".." as the first two directories.
        if ((ffd.cFileName) && (0 != strcmp(ffd.cFileName, ".")) && (0 != strcmp(ffd.cFileName, "..")))
        {
            std::shared_ptr<FileInfo> file_info = std::make_shared<FileInfo>();
            file_info->name_ = ffd.cFileName;
            file_info->relative_name_ = OsPathJoin(parent->relative_name_, file_info->name_);
            file_info->total_size_ = (static_cast<size_t>(ffd.nFileSizeHigh) << 32) | static_cast<size_t>(ffd.nFileSizeLow);

            uint64_t total_us = ((static_cast<uint64_t>(ffd.ftLastWriteTime.dwHighDateTime) << 32) |
                (static_cast<uint64_t>(ffd.ftLastWriteTime.dwLowDateTime))) / 10 - 11644473600000000ull;
            file_info->modify_timestamp_ = static_cast<int64_t>(total_us / 1000000);

            if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                file_info->file_mode_ = FileModes::kDirectory;
                FindChildFileInfo(file_info.get());
            }
            else
            {
                file_info->file_mode_ = FileModes::kRegularFile;
            }

            parent->total_size_ += file_info->total_size_;
            parent->childs_.push_back(file_info);
        }
    } while (FindNextFileA(file_handle, &ffd)); //Find the next file.
    FindClose(file_handle);
}

bool MkdirOs(const std::string &file_name)
{
    return ((0 == mkdir(file_name.c_str())) || (EEXIST == errno));
}

void RmOs(const std::string &file_name, bool b_top_dir)
{
    std::string search_name = file_name;
    if (!b_top_dir)
    {
        // Prepare string for use with FindFile functions.  First, copy the
        // string to a buffer, then append '\*' to the directory name.
        search_name += "\\*";
    }

    // Find the first file in the directory.
    WIN32_FIND_DATAA ffd;
    HANDLE file_handle = FindFirstFileA(search_name.c_str(), &ffd);
    if (INVALID_HANDLE_VALUE == file_handle)
    {
        return;
    }

    do
    {
        // Find first file will always return "." and ".." as the first two directories.
        if ((ffd.cFileName) && (0 != strcmp(ffd.cFileName, ".")) && (0 != strcmp(ffd.cFileName, "..")))
        {
            std::string full_name = (b_top_dir) ? file_name : OsPathJoin(file_name, ffd.cFileName);
            if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                RmOs(full_name, false);
                rmdir(full_name.c_str());
            }
            else
            {
                remove(full_name.c_str());
            }
        }
    } while (FindNextFileA(file_handle, &ffd)); // Find next file.
    FindClose(file_handle);
}

LCCL_FILE_END_NAMESPACE
LCCL_END_NAMESPACE

#endif
