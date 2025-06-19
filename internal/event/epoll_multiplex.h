#ifndef LCCL_INTERNAL_EVENT_EPOLL_MULTIPLEX_H_
#define LCCL_INTERNAL_EVENT_EPOLL_MULTIPLEX_H_

#if defined(_MSC_VER)
#else
#include <sys/epoll.h>
#include <unordered_map>
#include <vector>
#include "lccl/event.h"

LCCL_BEGIN_NAMESPACE
LCCL_EVENT_BEGIN_NAMESPACE

class EpollMultiplex : public IMultiplex
{
public:
    EpollMultiplex() = delete;
    EpollMultiplex(const EpollMultiplex &) = delete;
    EpollMultiplex &operator=(const EpollMultiplex &) = delete;

    explicit EpollMultiplex(int timeout_ms);
    virtual ~EpollMultiplex();

    virtual void RegisterHandler(int fd, int register_events, HandlerCallback callback);
    virtual void RemoveHandler(int fd);

    virtual void EnableEvent(int fd, int events);
    virtual void DisableEvent(int fd, int events);

    virtual void SetTimeoutMs(int timeout_ms);

    virtual int ExecuteOnce();

private:
    struct Handler
    {
        HandlerCallback callback = nullptr;
        int register_events = 0;
    };

private:
    int timeout_ms_;
    std::unordered_map<int, std::shared_ptr<Handler>> handlers_;

    int epoll_fd_;
    std::vector<epoll_event> epoll_events_;
};

LCCL_EVENT_END_NAMESPACE
LCCL_END_NAMESPACE

#endif

#endif // !LCCL_INTERNAL_EVENT_EPOLL_MULTIPLEX_H_
