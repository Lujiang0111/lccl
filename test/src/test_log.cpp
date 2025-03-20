#include <iostream>
#include <thread>
#include <vector>
#include "lccl/log.h"
#include "test_log.h"

enum class TestLogTypes
{
    kTestLog = 0,
    kTestZLog,
};

extern bool app_running;

void TestLogThread(lccl::log::ILogger *logger, int repeat_times, bool on_screen, bool sync)
{
    int test_no = 0;
    while (app_running)
    {
        ++test_no;
        for (int i = 0; i < repeat_times; ++i)
        {
            logger->LogFmt(lccl::log::Levels::kDebug, on_screen, sync, __FILE__, __LINE__, "test no {}, repeat {}", test_no, i);
            logger->LogFmt(lccl::log::Levels::kInfo, on_screen, sync, __FILE__, __LINE__, "test no {}, repeat {}", test_no, i);
            logger->LogFmt(lccl::log::Levels::kWarn, on_screen, sync, __FILE__, __LINE__, "test no {}, repeat {}", test_no, i);
            logger->LogFmt(lccl::log::Levels::kError, on_screen, sync, __FILE__, __LINE__, "test no {}, repeat {}", test_no, i);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void TestLog(int argc, char **argv)
{
    if (argc <= 6)
    {
        std::cerr << "too less args";
        return;
    }

    std::shared_ptr<lccl::log::ILogger> logger = lccl::log::CreateLogger("log",
        static_cast<size_t>(100) << 20,
        lccl::log::CompressTypes::kNone);

    std::shared_ptr<lccl::log::ILogger> zlogger = lccl::log::CreateLogger("zlog",
        static_cast<size_t>(100) << 20,
        lccl::log::CompressTypes::kGzip);
    
    TestLogTypes test_log_type = static_cast<TestLogTypes>(atoi(argv[2]));
    int thread_cnt = atoi(argv[3]);
    int repeat_times = atoi(argv[4]);
    bool on_screen = ('0' != *argv[5]);
    bool sync = ('0' != *argv[6]);

    std::vector<std::thread> test_threads;
    switch (test_log_type)
    {
    case TestLogTypes::kTestLog:
        for (size_t i = 0; i < thread_cnt; ++i)
        {
            test_threads.emplace_back(TestLogThread, logger.get(), repeat_times, on_screen, sync);
        }
        break;
    case TestLogTypes::kTestZLog:
        for (size_t i = 0; i < thread_cnt; ++i)
        {
            test_threads.emplace_back(TestLogThread, zlogger.get(), repeat_times, on_screen, sync);
        }
        break;
    default:
        break;
    }

    for (auto &&test_thread : test_threads)
    {
        if (test_thread.joinable())
        {
            test_thread.join();
        }
    }
}
