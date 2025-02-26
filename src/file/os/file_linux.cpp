#if defined(__linux__)

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "lccl/utils/path.h"
#include "file/file_info.h"
#include "log/lib_log.h"

LCCL_BEGIN_NAMESPACE
LCCL_FILE_BEGIN_NAMESPACE

std::shared_ptr<FileInfo> FileInfo::FindFileInfo(const std::string &file_name)
{
    struct stat file_stat;
    if (0 != lstat(file_name.c_str(), &file_stat))
    {
        LIB_LOG(log::Levels::kError, "lstat file [{}] fail! errno={}", file_name, strerror(errno));
        return nullptr;
    }

    std::shared_ptr<FileInfo> file_info = std::make_shared<FileInfo>();
    const char *p_split = strrchr(file_name.c_str(), kDirSep);
    if (p_split)
    {
        file_info->name_ = (p_split + 1);
    }
    else
    {
        file_info->name_ = file_name;
    }

    file_info->relative_name_ = file_name;
    file_info->total_size_ = file_stat.st_size;
    file_info->modify_timestamp_ = file_stat.st_ctime;

    if (S_ISDIR(file_stat.st_mode))
    {
        file_info->file_mode_ = FileModes::kDirectory;

        DIR *dir = opendir(file_name.c_str());
        if (!dir)
        {
            LIB_LOG(log::Levels::kError, "open dir [{}] fail! errno={}", file_name, strerror(errno));
            return nullptr;
        }

        dirent *p_dirent = nullptr;
        while (nullptr != (p_dirent = readdir(dir)))
        {
            if ((p_dirent->d_name) && (0 != strcmp(p_dirent->d_name, ".")) && (0 != strcmp(p_dirent->d_name, "..")))
            {
                std::string full_name = OsPathJoin(file_name, p_dirent->d_name);
                auto child = FindFileInfo(full_name);

                if (child)
                {
                    file_info->total_size_ += child->total_size_;
                    file_info->childs_.push_back(std::move(child));
                }
            }
        }
        closedir(dir);
    }
    else
    {
        file_info->file_mode_ = FileModes::kRegularFile;
    }

    return file_info;
}

bool MkdirOs(const std::string &file_name)
{
    return ((mkdir(file_name.c_str(), mode_t(0755)) == 0) || (EEXIST == errno));
}

void RmOs(const std::string &file_name, bool b_top_dir)
{
    struct stat file_stat;
    if (0 != lstat(file_name.c_str(), &file_stat))
    {
        return;
    }

    if (S_ISDIR(file_stat.st_mode))
    {
        DIR *dir = opendir(file_name.c_str());
        if (!dir)
        {
            return;
        }

        dirent *p_dirent = nullptr;
        while (nullptr != (p_dirent = readdir(dir)))
        {
            if ((p_dirent->d_name) && (0 != strcmp(p_dirent->d_name, ".")) && (0 != strcmp(p_dirent->d_name, "..")))
            {
                std::string full_name = OsPathJoin(file_name, p_dirent->d_name);
                RmOs(full_name, false);
            }
        }
        closedir(dir);

        rmdir(file_name.c_str());
    }
    else
    {
        remove(file_name.c_str());
    }
}

LCCL_FILE_END_NAMESPACE
LCCL_END_NAMESPACE

#endif
