#ifndef LCCL_INTERNAL_FILE_FILE_MMAP_H_
#define LCCL_INTERNAL_FILE_FILE_MMAP_H_

#include <string>
#include <vector>
#include "lccl/file.h"

LCCL_NAMESPACE_BEGIN
LCCL_FILE_NAMESPACE_BEGIN

class FileMmap : public IFileMmap
{
public:
    FileMmap(const FileMmap &) = delete;
    FileMmap &operator=(const FileMmap &) = delete;

    FileMmap();
    virtual ~FileMmap();

    bool Init(const std::string &file_name);
    void Deinit();

    virtual const uint8_t *GetData() const;
    virtual size_t GetSize() const;

private:
#if defined(_MSC_VER)
    void *handle_;
    void *mapping_handle_;
#else
    int fd_;
#endif
    void *data_;
    size_t size_;
};

LCCL_FILE_NAMESPACE_END
LCCL_NAMESPACE_END

#endif // !LCCL_INTERNAL_FILE_FILE_MMAP_H_
