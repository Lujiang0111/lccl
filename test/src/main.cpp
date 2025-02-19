#include <cstring>
#include <iostream>
#include "test_file.h"
#include "test_fmt.h"

enum class TestTypes
{
    kTestFile,
    kTestFmt,
};

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        std::cerr << "too less args";
        return -1;
    }

    bool ret = true;
    TestTypes test_type = static_cast<TestTypes>(atoi(argv[1]));
    switch (test_type)
    {
    case TestTypes::kTestFile:
        ret = TestFile(argc, argv);
        break;
    case TestTypes::kTestFmt:
        ret = TestFmt(argc, argv);
        break;
    default:
        ret = false;
        break;
    }
    
    if (!ret)
    {
        return -1;
    }
    return 0;
}
