#include "lccl/utils/path.h"
#include "file/file_os.h"
#include "log/lib_log.h"

LCCL_BEGIN_NAMESPACE
LCCL_FILE_BEGIN_NAMESPACE

bool CreateDir(const char *file_name, bool parent_only)
{
    if (!file_name)
    {
        return true;
    }

    std::string adj_file_name = OsPathJoin(file_name);

    size_t search_offset = 0;
#if defined(_MSC_VER)
    if ((adj_file_name.length() >= 3) && (':' == adj_file_name[1]) && (kDirSep == adj_file_name[2]))
    {
        search_offset = 3;
    }
#else
    if (kDirSep == file_name[0])
    {
        search_offset = 1;
    }
#endif

    do
    {
        auto token_pos = adj_file_name.find_first_of(kDirSep, search_offset);
        // treat the entire path as a folder if no folder separator not found
        if (std::string::npos == token_pos)
        {
            if (parent_only)
            {
                break;
            }

            token_pos = adj_file_name.length();
        }

        auto subdir_name = adj_file_name.substr(0, token_pos);
        if ((!subdir_name.empty()) && (!MkdirOs(subdir_name)))
        {
            LIB_LOG(log::Levels::kError, "create dir [{}] fail! errno={}", subdir_name, strerror(errno));
            return false;
        }
        search_offset = token_pos + 1;
    } while (search_offset < adj_file_name.length());

    return true;
}

void RemoveFile(const char *file_name)
{
    std::string adj_file_name = OsPathJoin(file_name);
    RmOs(adj_file_name, true);
}

LCCL_FILE_END_NAMESPACE
LCCL_END_NAMESPACE
