#if defined(_MSC_VER)
#include <windows.h>
#else
#include <sys/syscall.h>
#endif

#include "lccl/fmt.h"
#include "lccl/utils/path.h"
#include "log/lib_log.h"
#include "log/logger.h"

LCCL_BEGIN_NAMESPACE
LCCL_LOG_BEGIN_NAMESPACE

constexpr size_t kZlibChunkSize = 16 * 1024;

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
    exit_flag_(false),
    log_thread_running_(false),
    enq_log_msgs_(10),
    deq_log_msgs_(10),
    enq_log_msgs_full_(false),
    file_hour_count_(0),
    file_write_size_(0),
    z_inbuf_(kZlibChunkSize),
    z_inbuf_size_(0),
    z_outbuf_(kZlibChunkSize)
{
    utils_ = &Utils::Instance();
    memset(&z_stream_, 0, sizeof(z_stream_));

    log_thread_running_ = true;
    log_thread_ = std::thread(&Logger::LogThread, this);
#if defined(_MSC_VER)
#else
    pthread_setname_np(log_thread_.native_handle(), "log");
#endif
}

Logger::~Logger()
{
    exit_flag_ = true;
    std::shared_ptr<LogMsg> log_msg = std::make_shared<LogMsg>();
    EnqueueLogMsg(log_msg);

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
    std::shared_ptr<LogMsg> log_msg = std::make_shared<LogMsg>();
    log_msg->time_point = std::chrono::system_clock::now();
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
        if (EnqueueLogMsg(log_msg))
        {
            future.get();
        }
    }
    else
    {
        EnqueueLogMsg(log_msg);
    }
}

bool Logger::EnqueueLogMsg(std::shared_ptr<LogMsg> log_msg)
{
    std::lock_guard<std::mutex> lock(log_msgs_mutex_);
    if (exit_flag_)
    {
        log_thread_running_ = false;
        return false;
    }

    if (!enq_log_msgs_.Push(log_msg))
    {
        enq_log_msgs_full_ = true;
        return false;
    }
    
    return true;
}

bool Logger::DequeueLogMsgs()
{
    if ((!deq_log_msgs_.Empty()) || (enq_log_msgs_.Empty()))
    {
        return false;
    }

    bool enq_log_msgs_full = enq_log_msgs_full_;
    {
        std::lock_guard<std::mutex> lock(log_msgs_mutex_);
        enq_log_msgs_.Swap(deq_log_msgs_);
        enq_log_msgs_full_ = false;
    }

    if (enq_log_msgs_full)
    {
        std::shared_ptr<LogMsg> log_msg = nullptr;
        while (deq_log_msgs_.Pop(log_msg))
        {
            if (log_msg->sync_promise)
            {
                log_msg->sync_promise->set_value(false);
            }
        }
    }

    return true;
}

void Logger::LogThread()
{
    size_t id = 0;
    std::shared_ptr<LogMsg> log_msg = nullptr;
    while (true)
    {
        if (deq_log_msgs_.Empty())
        {
            if (!DequeueLogMsgs())
            {
                if (!log_thread_running_)
                {
                    break;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
        }

        deq_log_msgs_.Pop(log_msg);
        PrintLog(*log_msg, id++);

        if (log_msg->sync_promise)
        {
            log_msg->sync_promise->set_value(true);
        }
    }

    CloseLog();
}

void Logger::PrintLog(LogMsg &log_msg, size_t id)
{
    UpdateLogFile(log_msg.time_point);

    // get tm
    time_t time_tt = std::chrono::system_clock::to_time_t(log_msg.time_point);
    std::tm time_tm;
#if defined(_MSC_VER)
    ::localtime_s(&time_tm, &time_tt);
#else
    ::localtime_r(&time_tt, &time_tm);
#endif
    int msec = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(log_msg.time_point.time_since_epoch()).count() % 1000);
    auto &&level_map = utils_->GetLvelMap(log_msg.level);

    std::string content;
    if (log_msg.file_name.length() > 0)
    {
        if (log_msg.file_line > 0)
        {
            content = fmt::format("[{}], {:02d}:{:02d}:{:02d} {:03d}, {}, {}:{}, T:{}, {}\n",
                id, time_tm.tm_hour, time_tm.tm_min, time_tm.tm_sec, msec,
                level_map.str, log_msg.file_name, log_msg.file_line, log_msg.thread_id, log_msg.content);
        }
        else
        {
            content = fmt::format("[{}], {:02d}:{:02d}:{:02d} {:03d}, {}, {}, T:{}, {}\n",
                id, time_tm.tm_hour, time_tm.tm_min, time_tm.tm_sec, msec,
                level_map.str, log_msg.file_name, log_msg.thread_id, log_msg.content);
        }
    }
    else
    {
        content = fmt::format("[{}], {:02d}:{:02d}:{:02d} {:03d}, {}, T:{}, {}\n",
            id, time_tm.tm_hour, time_tm.tm_min, time_tm.tm_sec, msec,
            level_map.str, log_msg.thread_id, log_msg.content);
    }

    if (log_msg.on_screen)
    {
        fmt::print(fmt::fg(level_map.fg_color), "{}", content);
    }

    if (!fout_.is_open())
    {
        return;
    }

    switch (compress_type_)
    {
    case CompressTypes::kNone:
    {
        fout_ << content;
        fout_.flush();
        file_write_size_ += content.length();
        break;
    }

    case CompressTypes::kGzip:
    {
        if (z_inbuf_size_ + content.size() > z_inbuf_.size())
        {
            z_inbuf_.resize(z_inbuf_.size() + content.size() + kZlibChunkSize);
        }
        memcpy(&z_inbuf_[0] + z_inbuf_size_, content.c_str(), content.size());
        z_inbuf_size_ += content.size();

        if (z_inbuf_size_ >= kZlibChunkSize)
        {
            z_stream_.avail_in = static_cast<uInt>(z_inbuf_size_);
            z_stream_.next_in = &z_inbuf_[0];

            do
            {
                z_stream_.avail_out = kZlibChunkSize;
                z_stream_.next_out = &z_outbuf_[0];

                if (Z_STREAM_ERROR == deflate(&z_stream_, Z_PARTIAL_FLUSH))
                {
                    fout_.close();
                    return;
                }

                fout_.write(reinterpret_cast<const char *>(&z_outbuf_[0]), kZlibChunkSize - z_stream_.avail_out);
                fout_.flush();
                file_write_size_ += (kZlibChunkSize - z_stream_.avail_out);
            } while (0 == z_stream_.avail_out);

            z_inbuf_size_ = 0;
        }
        break;
    }

    default:
        break;
    }
}

void Logger::CloseLog()
{
    if (fout_.is_open())
    {
        switch (compress_type_)
        {
        case CompressTypes::kGzip:
        {
            z_stream_.avail_in = static_cast<uInt>(z_inbuf_size_);
            z_stream_.next_in = &z_inbuf_[0];

            do
            {
                z_stream_.avail_out = kZlibChunkSize;
                z_stream_.next_out = &z_outbuf_[0];

                if (Z_STREAM_ERROR == deflate(&z_stream_, Z_FINISH))
                {
                    break;
                }
                fout_.write(reinterpret_cast<const char *>(&z_outbuf_[0]), kZlibChunkSize - z_stream_.avail_out);
            } while (0 == z_stream_.avail_out);
            deflateEnd(&z_stream_);
        }

        default:
            break;
        }

        fout_.close();
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
    if ((!fout_.is_open()) ||
        (hour_count != file_hour_count_) ||
        ((max_size_ > 0) && (file_info_) && (file_info_->GetTotalSize() + file_write_size_ > max_size_)))
    {
        // close exist log
        CloseLog();

        // do cleaning
        UpdateFileInfo();
        if ((max_size_ > 0) && (file_info_) && (file_info_->GetTotalSize() > max_size_))
        {
            size_t clear_size = file_info_->GetTotalSize() - max_size_ / 10 * 8;
            DeleteLogFile(file_info_.get(), clear_size);
            UpdateFileInfo();
        }

        // get tm
        time_t time_tt = std::chrono::system_clock::to_time_t(time_point);
        std::tm time_tm;
#if defined(_MSC_VER)
        ::localtime_s(&time_tm, &time_tt);
#else
        ::localtime_r(&time_tt, &time_tm);
#endif

        // create new file
        std::string file_name;
        switch (compress_type_)
        {
        case CompressTypes::kNone:
        {
            file_name = fmt::format("{}{}{:04d}-{:02d}-{:02d}{}{:04d}-{:02d}-{:02d}-{:02d}.log",
                path_, kDirSep,
                time_tm.tm_year + 1900, time_tm.tm_mon + 1, time_tm.tm_mday, kDirSep,
                time_tm.tm_year + 1900, time_tm.tm_mon + 1, time_tm.tm_mday, time_tm.tm_hour);

            file::CreateDir(file_name.c_str(), true);
            fout_.open(file_name.c_str(), std::ios::app);
            break;
        }

        case CompressTypes::kGzip:
        {
            // 初始化zlib
            int ret = deflateInit2(&z_stream_, Z_DEFAULT_COMPRESSION, Z_DEFLATED, MAX_WBITS + 16, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
            if (Z_OK != ret)
            {
                LIB_LOG(Levels::kError, "deflateInit2 fail! ret={}", ret);
                break;
            }

            for (int file_no = 0; ; ++file_no)
            {
                file_name = fmt::format("{}{}{:04d}-{:02d}-{:02d}{}{:04d}-{:02d}-{:02d}-{:02d}{}.log.gz",
                    path_, kDirSep,
                    time_tm.tm_year + 1900, time_tm.tm_mon + 1, time_tm.tm_mday, kDirSep,
                    time_tm.tm_year + 1900, time_tm.tm_mon + 1, time_tm.tm_mday, time_tm.tm_hour,
                    (0 == file_no) ? "" : fmt::format("-{}", file_no));
                auto file_info = file::CreateFileInfo(file_name.c_str(), file::SortTypes::kByModifyTime);
                if (!file_info)
                {
                    break;
                }
            }

            file::CreateDir(file_name.c_str(), true);
            fout_.open(file_name.c_str(), std::ios::binary);
            break;
        }

        default:
            break;
        }

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
