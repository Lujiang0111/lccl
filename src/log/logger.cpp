#if defined(_MSC_VER)
#include <windows.h>
#else
#endif

#include "lccl/fmt.h"
#include "lccl/utils/path.h"
#include "log/logger.h"

LCCL_BEGIN_NAMESPACE
LCCL_LOG_BEGIN_NAMESPACE

static inline size_t GetThreadId_()
{
#if defined(_MSC_VER)
    return static_cast<size_t>(::GetCurrentThreadId());
#elif defined(__linux__)
#    if defined(__ANDROID__) && defined(__ANDROID_API__) && (__ANDROID_API__ < 21)
#        define SYS_gettid __NR_gettid
#    endif
    return static_cast<size_t>(::syscall(SYS_gettid));
#elif defined(_AIX) || defined(__DragonFly__) || defined(__FreeBSD__)
    return static_cast<size_t>(::pthread_getthreadid_np());
#elif defined(__NetBSD__)
    return static_cast<size_t>(::_lwp_self());
#elif defined(__OpenBSD__)
    return static_cast<size_t>(::getthrid());
#elif defined(__sun)
    return static_cast<size_t>(::thr_self());
#elif __APPLE__
    uint64_t tid;
    pthread_threadid_np(nullptr, &tid);
    return static_cast<size_t>(tid);
#else // Default to standard C++11 (other Unix)
    return static_cast<size_t>(std::hash<std::thread::id>()(std::this_thread::get_id()));
#endif
}

static inline size_t GetThreadId()
{
    static thread_local const size_t tid = GetThreadId_();
    return tid;
}

Logger::Logger(const std::string &path, size_t max_size, CompressTypes compress_type) :
    path_(path),
    max_size_(max_size),
    compress_type_(compress_type),
    max_level_(Levels::kNb),
    utils_(nullptr),
    log_thread_running_(false),
    log_msgs_(10),
    file_hour_count_(0),
    file_write_size_(0)
{
    utils_ = &Utils::Instance();

    log_thread_running_ = true;
    log_thread_ = std::thread(&Logger::LogThread, this);
#if defined(_MSC_VER)
#else
    pthread_setname_np(log_thread_.native_handle(), "log");
#endif
}

Logger::~Logger()
{
    log_thread_running_ = false;
    if (log_thread_.joinable())
    {
        log_thread_.join();
    }
}

void Logger::SetMaxLevel(Levels max_level)
{
    max_level_ = max_level;
}

void Logger::LogContent(Levels level, bool on_screen, bool sync, const char *file_name, int file_line, const char *content)
{
    if (!log_thread_running_)
    {
        return;
    }

    std::shared_ptr<LogMsg> log_msg = std::make_shared<LogMsg>();
    log_msg->time = std::chrono::system_clock::now();
    log_msg->thread_id = GetThreadId();
    log_msg->level = level;
    log_msg->on_screen = on_screen;
    log_msg->file_name = (file_name) ? file_name : "";
    log_msg->file_line = file_line;
    log_msg->content = (content) ? content : "";

    if (sync)
    {
        log_msg->sync_promise = std::make_shared<std::promise<bool>>();
        std::future<bool> future = log_msg->sync_promise->get_future();
        if (log_msgs_.Push(log_msg))
        {
            future.get();
        }
    }
    else
    {
        log_msgs_.Push(log_msg);
    }
}

void Logger::LogThread()
{
    size_t id = 0;
    std::shared_ptr<LogMsg> log_msg = nullptr;
    while ((log_thread_running_) || (!log_msgs_.Empty()))
    {
        if (!log_msgs_.Pop(log_msg))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        PrintLog(*log_msg, id++);
    }
}

void Logger::PrintLog(LogMsg &log_msg, size_t id)
{
    UpdateLogFile(log_msg.time);

    // get tm
    time_t time_tt = std::chrono::system_clock::to_time_t(log_msg.time);
    std::tm time_tm;
#if defined(_MSC_VER)
    ::localtime_s(&time_tm, &time_tt);
#else
    ::localtime_r(&time_tt, &time_tm);
#endif
    int msec = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(log_msg.time.time_since_epoch()).count() % 1000);
    auto &&level_map = utils_->GetLvelMap(log_msg.level);

    std::string content;
    if (log_msg.file_name.length() > 0)
    {
        if (log_msg.file_line > 0)
        {
            content = fmt::format("[{}], {:02d}:{:02d}:{:02d} {:03d}, {}, {}:{}, T:{}, {}",
                id, time_tm.tm_hour, time_tm.tm_min, time_tm.tm_sec, msec,
                level_map.str, log_msg.file_name, log_msg.file_line, log_msg.thread_id, log_msg.content);
        }
        else
        {
            content = fmt::format("[{}], {:02d}:{:02d}:{:02d} {:03d}, {}, {}, T:{}, {}",
                id, time_tm.tm_hour, time_tm.tm_min, time_tm.tm_sec, msec,
                level_map.str, log_msg.file_name, log_msg.thread_id, log_msg.content);
        }
    }
    else
    {
        content = fmt::format("[{}], {:02d}:{:02d}:{:02d} {:03d}, {}, T:{}, {}",
            id, time_tm.tm_hour, time_tm.tm_min, time_tm.tm_sec, msec,
            level_map.str, log_msg.thread_id, log_msg.content);
    }

    switch (compress_type_)
    {
    case CompressTypes::kNone:
    {
        if (log_msg.on_screen)
        {
            fmt::print(fmt::fg(level_map.fg_color), "{}\n", content);
        }

        if (fout_.is_open())
        {
            fout_ << content << std::endl;
            fout_.flush();
            file_write_size_ += content.length();
        }
        break;
    }

    case CompressTypes::kGzip:
        break;
    default:
        break;
    }
}

void Logger::UpdateFileInfo()
{
    file::CreateDir(path_.c_str(), false);
    file_info_ = file::CreateFileInfo(path_.c_str(), file::SortTypes::kByModifyTime);
}

void Logger::UpdateLogFile(std::chrono::system_clock::time_point time_point)
{
    int64_t hour_count = std::chrono::duration_cast<std::chrono::hours>(time_point.time_since_epoch()).count();
    if ((hour_count != file_hour_count_) ||
        ((max_size_ > 0) && (file_info_) && (file_info_->GetTotalSize() + file_write_size_ > max_size_)))
    {
        // get tm
        time_t time_tt = std::chrono::system_clock::to_time_t(time_point);
        std::tm time_tm;
#if defined(_WIN32)
        ::localtime_s(&time_tm, &time_tt);
#else
        ::localtime_r(&time_tt, &time_tm);
#endif

        // close exist file
        if (fout_.is_open())
        {
            fout_.close();
        }

        // do cleaning
        UpdateFileInfo();
        if ((max_size_ > 0) && (file_info_) && (file_info_->GetTotalSize() > max_size_))
        {
            size_t clear_size = file_info_->GetTotalSize() - max_size_ / 10 * 8;
            DeleteLogFile(file_info_.get(), clear_size);
            UpdateFileInfo();
        }

        // create new file
        std::string file_name = fmt::format("{}{}{:04d}-{:02d}-{:02d}{}{:04d}-{:02d}-{:02d}-{:02d}.log",
            path_, kDirSep,
            time_tm.tm_year + 1900, time_tm.tm_mon + 1, time_tm.tm_mday, kDirSep,
            time_tm.tm_year + 1900, time_tm.tm_mon + 1, time_tm.tm_mday, time_tm.tm_hour);
        file::CreateDir(file_name.c_str(), true);
        fout_.open(file_name.c_str(), std::ios::app);

        file_hour_count_ = hour_count;
        file_write_size_ = 0;
    }
}

void Logger::DeleteLogFile(const file::IFileInfo *file_info, size_t &del_size)
{
    if (0 == del_size)
    {
        return;
    }

    if ((file::FileModes::kRegularFile == file_info->GetFileMode()) ||
        (file_info->GetTotalSize() < del_size))
    {
        file::RemoveFile(file_info->GetRelativeName());
        del_size = (del_size > file_info->GetTotalSize()) ? (del_size - file_info->GetTotalSize()) : 0;
    }
    else if (file::FileModes::kDirectory == file_info->GetFileMode())
    {
        for (size_t i = 0; i < file_info->GetChildCnt(); ++i)
        {
            DeleteLogFile(file_info->GetChild(i), del_size);
        }
    }
}

std::shared_ptr<ILogger> CreateLogger(const char *path, size_t max_size, CompressTypes compress_type)
{
    std::shared_ptr<Logger> logger = std::make_shared<Logger>((path) ? path : "", max_size, compress_type);
    return logger;
}

ILogger *DefaultLogger()
{
    static std::shared_ptr<ILogger> default_logger = CreateLogger("Log", 0, CompressTypes::kNone);
    return default_logger.get();
}

LCCL_LOG_END_NAMESPACE
LCCL_END_NAMESPACE
