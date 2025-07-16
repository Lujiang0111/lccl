#include <csignal>
#include <iostream>
#include "lccl.h"
#include "test_fmt.h"
#include "test_file.h"
#include "test_log.h"
#include "test_ring_buffer.h"

enum class TestTypes
{
    kTestVersion = 0,
    kTestFmt,
    kTestFile,
    kTestLog,
    kTestRingBuffer,
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
    case TestTypes::kTestFile:
        TestFile(argc, argv);
        break;
    case TestTypes::kTestLog:
        TestLog(argc, argv);
        break;
    case TestTypes::kTestRingBuffer:
        TestRingBuffer(argc, argv);
        break;
    default:
        return -1;
    }

    return 0;
}
