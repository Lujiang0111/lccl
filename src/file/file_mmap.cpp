#if defined(_MSC_VER)
#include <Windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "file/file_mmap.h"
#include "log/lib_log.h"

LCCL_BEGIN_NAMESPACE
LCCL_FILE_BEGIN_NAMESPACE

FileMmap::FileMmap() :
#if defined(_MSC_VER)
    handle_(INVALID_HANDLE_VALUE),
    mapping_handle_(nullptr),
    data_(nullptr),
#else
    fd_(-1),
    data_(MAP_FAILED),
#endif
    size_(0)
{

}

FileMmap::~FileMmap()
{
    Deinit();
}

bool FileMmap::Init(const std::string &file_name)
{
#if defined(_MSC_VER)
    handle_ = CreateFile(file_name.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (INVALID_HANDLE_VALUE == handle_)
    {
        LIB_LOG(log::Levels::kError, "Can not open file={}", file_name);
        return false;
    }

    DWORD size_high = 0;
    DWORD size_low = GetFileSize(handle_, &size_high);
    size_ = (static_cast<size_t>(size_high) << 32) | static_cast<size_t>(size_low);

    mapping_handle_ = CreateFileMapping(handle_, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (!mapping_handle_)
    {
        LIB_LOG(log::Levels::kError, "Can not mapping file={}", file_name);
        return false;
    }

    data_ = MapViewOfFile(mapping_handle_, FILE_MAP_READ, 0, 0, 0);
#else
    fd_ = open(file_name.c_str(), O_RDONLY);
    if (fd_ < 0)
    {
        LIB_LOG(log::Levels::kError, "Can not open file={}", file_name);
        return false;
    }

    struct stat st = { 0 };
    if (fstat(fd_, &st) < 0)
    {
        LIB_LOG(log::Levels::kError, "Get file stat fail, file={}", file_name);
        return false;
    }

    size_ = st.st_size;

    data_ = mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, fd_, 0);
    if (MAP_FAILED == data_)
    {
        LIB_LOG(log::Levels::kError, "Map file fail, file={}", file_name);
        return false;
    }

    // mmap 后可立即关闭文件
    close(fd_);
    fd_ = -1;
#endif

    return true;
}

void FileMmap::Deinit()
{
#if defined(_MSC_VER)
    if (data_)
    {
        UnmapViewOfFile(data_);
        data_ = nullptr;
    }

    if (mapping_handle_)
    {
        CloseHandle(mapping_handle_);
        mapping_handle_ = nullptr;
    }

    if (INVALID_HANDLE_VALUE != handle_)
    {
        CloseHandle(handle_);
        handle_ = INVALID_HANDLE_VALUE;
    }
#else
    if (MAP_FAILED != data_)
    {
        munmap(data_, size_);
        data_ = MAP_FAILED;
    }

    if (fd_ >= 0)
    {
        close(fd_);
        fd_ = -1;
    }
#endif
}

const uint8_t *FileMmap::GetData() const
{
    return static_cast<uint8_t *>(data_);
}

size_t FileMmap::GetSize() const
{
    return size_;
}

std::shared_ptr<IFileMmap> CreateFileMmap(const char *file_name)
{
    if (!file_name)
    {
        return nullptr;
    }

    std::shared_ptr<FileMmap> h = std::make_shared<FileMmap>();
    if (!h->Init(file_name))
    {
        return false;
    }

    return h;
}

LCCL_FILE_END_NAMESPACE
LCCL_END_NAMESPACE
