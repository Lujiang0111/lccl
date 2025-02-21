#include <csignal>
#include <iostream>
#include <thread>
#include "lccl.h"
#include "test_fmt.h"

enum class TestTypes
{
    kTestVersion = 0,
    kTestFmt,
};

bool app_running = true;

static void SigIntHandler(int sig_num)
{
    signal(SIGINT, SigIntHandler);
    app_running = false;
}

int main(int argc, char **argv)
{
    signal(SIGINT, SigIntHandler);

    if (argc <= 1)
    {
        std::cerr << "too less args";
        return -1;
    }

    TestTypes test_type = static_cast<TestTypes>(atoi(argv[1]));
    switch (test_type)
    {
    case TestTypes::kTestVersion:
        std::cout << "lccl version " << lccl::GetVersion();
        break;
    case TestTypes::kTestFmt:
        TestFmt(argc, argv);
        break;
    default:
        break;
    }

    return 0;
}
