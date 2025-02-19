#include <csignal>
#include <cstring>
#include <iostream>
#include <thread>
#include "test_file.h"
#include "test_fmt.h"
#include "test_log.h"

enum class TestTypes
{
    kTestFile = 0,
    kTestFmt,
    kTestLog,
};

bool app_running = true;

#if defined(_MSC_VER)
void DelayExit()
{
    std::this_thread::sleep_for(std::chrono::seconds(10));
    app_running = false;
}
#else
static void SigIntHandler(int sig_num)
{
    signal(SIGINT, SigIntHandler);
    app_running = false;
}
#endif

int main(int argc, char **argv)
{
#if defined(_MSC_VER)
    std::thread delay_exit_thread(DelayExit);
#else
    signal(SIGINT, SigIntHandler);
#endif

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
    case TestTypes::kTestLog:
        ret = TestLog(argc, argv);
        break;
    default:
        ret = false;
        break;
    }

#if defined(_MSC_VER)
    if (delay_exit_thread.joinable())
    {
        delay_exit_thread.join();
    }
#endif

    if (!ret)
    {
        return -1;
    }
    return 0;
}
