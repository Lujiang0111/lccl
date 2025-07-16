#ifndef LCCL_INTERNAL_LOG_LOGGER_H_
#define LCCL_INTERNAL_LOG_LOGGER_H_

#include <atomic>
#include <fstream>
#include <future>
#include <mutex>
#include <thread>
#include "zlib.h"
#include "lccl/file.h"
#include "lccl/log.h"
#include "lccl/oss/concurrentqueue/concurrentqueue.h"
#include "log/utils.h"

LCCL_NAMESPACE_BEGIN
LCCL_LOG_NAMESPACE_BEGIN

struct LogMsg
{
    std::chrono::system_clock::time_point time_point;
    size_t thread_id;
    Levels level;
    bool on_screen;
    std::string file_name;
    int file_line;
    std::string content;
    std::shared_ptr<std::promise<bool>> sync_promise;
};

class Logger : public ILogger
{
public:
    Logger() = delete;
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    Logger(const std::string &path, size_t max_size, CompressTypes compress_type);
    virtual ~Logger();

    virtual void SetMaxLevel(Levels max_level);
    virtual void LogContent(Levels level, bool on_screen, bool sync, const char *file_name, int file_line, const char *content);

private:
    bool EnqueueLogMsg(std::shared_ptr<LogMsg> log_msg);

    void LogThread();
    void PrintLog(LogMsg &log_msg, size_t id);
    void CloseLog();

    void UpdateFileInfo();
    void UpdateLogFile(std::chrono::system_clock::time_point time_point);
    void DeleteLogFile(const file::IFileInfo *file_info, size_t &del_size);

private:
    std::string path_;
    size_t max_size_;
    CompressTypes compress_type_;
    Levels max_level_;
    Utils *utils_;

    std::thread log_thread_;
    bool log_thread_running_;

    moodycamel::ConcurrentQueue<std::shared_ptr<LogMsg>> log_msgs_;

    std::ofstream fout_;
    std::shared_ptr<file::IFileInfo> file_info_;
    int64_t file_hour_count_;
    size_t file_write_size_;

    // zlog
    z_stream z_stream_;
    std::vector<uint8_t> z_inbuf_;
    size_t z_inbuf_size_;
    std::vector<uint8_t> z_outbuf_;
};

LCCL_LOG_NAMESPACE_END
LCCL_NAMESPACE_END

#endif // !LCCL_INTERNAL_LOG_LOGGER_H_
