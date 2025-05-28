#include <iostream>
#include <string>
#include "lccl/file.h"
#include "test_file.h"

enum class TestFileTypes
{
    kTestFileInfo = 0,
    kTestMmap,
};

static void PrintFileInfo(const lccl::file::IFileInfo *file_info, int depth)
{
    if (!file_info)
    {
        return;
    }

    for (int i = 0; i < depth; ++i)
    {
        std::cout << "    ";
    }
    std::cout << file_info->GetName() << " \033[33m"
        << file_info->GetTotalSize() << " " << file_info->GetModifyTimestamp() << "\033[0m" << std::endl;

    for (size_t i = 0; i < file_info->GetChildCnt(); ++i)
    {
        PrintFileInfo(file_info->GetChild(i), depth + 1);
    }
}

void TestFileInfo(int argc, char **argv)
{
    if (argc <= 3)
    {
        std::cerr << "too less args";
    }
    std::string file_name = argv[3];

    {
        auto file_info = lccl::file::CreateFileInfo(file_name.c_str(), lccl::file::SortTypes::kByName);
        if (!file_info)
        {
            std::cout << "can not find file info, file name=" << file_name << std::endl;
        }
        std::cout << std::endl << std::endl << "sort by name:" << std::endl;
        PrintFileInfo(file_info.get(), 0);
    }

    {
        auto file_info = lccl::file::CreateFileInfo(file_name.c_str(), lccl::file::SortTypes::kByModifyTime);
        if (!file_info)
        {
            std::cout << "can not find file info, file name=" << file_name << std::endl;
        }
        std::cout << std::endl << std::endl << "sort by modify time:" << std::endl;
        PrintFileInfo(file_info.get(), 0);
    }
}

void TestMmap(int argc, char **argv)
{
    if (argc <= 3)
    {
        std::cerr << "too less args";
    }
    std::string file_name = argv[3];

    std::shared_ptr<lccl::file::IFileMmap> file_mmap = lccl::file::CreateFileMmap(file_name.c_str());
    if (!file_mmap)
    {
        std::cout << "can not create file mmap, file name=" << file_name << std::endl;
    }

    std::cout << "file data = " << file_mmap->GetData() << std::endl;
    std::cout << "file size = " << file_mmap->GetSize() << std::endl;
}

void TestFile(int argc, char **argv)
{
    if (argc <= 2)
    {
        std::cerr << "too less args";
    }
    TestFileTypes test_file_type = static_cast<TestFileTypes>(atoi(argv[2]));

    switch (test_file_type)
    {
    case TestFileTypes::kTestFileInfo:
        TestFileInfo(argc, argv);
        break;
    case TestFileTypes::kTestMmap:
        TestMmap(argc, argv);
        break;
    default:
        break;
    }
}
