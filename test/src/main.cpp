#include <cstring>
#include <iostream>
#include "test_fmt.h"

enum class TestTypes
{
    kTestFmt = 0,
};

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        std::cerr << "too less args";
    }

    TestTypes test_type = static_cast<TestTypes>(atoi(argv[1]));
    switch (test_type)
    {
    case TestTypes::kTestFmt:
        TestFmt(argc, argv);
        break;
    default:
        break;
    }
    
    return 0;
}
