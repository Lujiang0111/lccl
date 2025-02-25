#include <iostream>
#include <string>
#include "lccl/file.h"
#include "test_file.h"

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

void TestFile(int argc, char **argv)
{
    if (argc <= 2)
    {
        std::cerr << "too less args";
    }
    std::string file_name = argv[2];

    {
        auto file_info = lccl::file::CreateFileInfo(file_name.c_str(), lccl::file::SortTypes::kByName);
        if (!file_info)
        {
            std::cout << "can not file file info, file name=" << file_name << std::endl;
        }
        std::cout << std::endl << std::endl << "sort by name:" << std::endl;
        PrintFileInfo(file_info.get(), 0);
    }
    
    {
        auto file_info = lccl::file::CreateFileInfo(file_name.c_str(), lccl::file::SortTypes::kByModifyTime);
        if (!file_info)
        {
            std::cout << "can not file file info, file name=" << file_name << std::endl;
        }
        std::cout << std::endl << std::endl << "sort by modify time:" << std::endl;
        PrintFileInfo(file_info.get(), 0);
    }
}
