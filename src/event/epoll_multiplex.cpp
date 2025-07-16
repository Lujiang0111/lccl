#if defined(_MSC_VER)
#else
#include <unistd.h>
#include "lccl/socket.h"
#include "event/epoll_multiplex.h"

LCCL_NAMESPACE_BEGIN
LCCL_EVENT_NAMESPACE_BEGIN

EpollMultiplex::EpollMultiplex(int timeout_ms) :
    timeout_ms_(timeout_ms)
{
    epoll_fd_ = epoll_create1(0);
}

EpollMultiplex::~EpollMultiplex()
{
    if (epoll_fd_ >= 0)
    {
        close(epoll_fd_);
        epoll_fd_ = -1;
    }
}

void EpollMultiplex::RegisterHandler(int fd, int register_events, HandlerCallback callback)
{
    std::shared_ptr<Handler> handler = std::make_shared<Handler>();
    handler->callback = callback;
    handler->register_events = register_events;

    RemoveHandler(fd);
    handlers_.emplace(fd, handler);

    epoll_event ev = { 0 };
    if (register_events & EventTypes::kRead)
    {
        ev.events |= EPOLLIN;
    }
    if (register_events & EventTypes::kWrite)
    {
        ev.events |= EPOLLOUT;
    }
    ev.data.fd = fd;

    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev);
}

void EpollMultiplex::RemoveHandler(int fd)
{
    auto iter = handlers_.find(fd);
    if (handlers_.end() == iter)
    {
        return;
    }

    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
    handlers_.erase(iter);
}

void EpollMultiplex::EnableEvent(int fd, int events)
{
    auto iter = handlers_.find(fd);
    if (handlers_.end() == iter)
    {
        return;
    }

    iter->second->register_events |= events;

    epoll_event ev = { 0 };
    if (iter->second->register_events & EventTypes::kRead)
    {
        ev.events |= EPOLLIN;
    }
    if (iter->second->register_events & EventTypes::kWrite)
    {
        ev.events |= EPOLLOUT;
    }
    ev.data.fd = fd;
    epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
}

void EpollMultiplex::DisableEvent(int fd, int events)
{
    auto iter = handlers_.find(fd);
    if (handlers_.end() == iter)
    {
        return;
    }

    iter->second->register_events &= ~events;

    epoll_event ev = { 0 };
    if (iter->second->register_events & EventTypes::kRead)
    {
        ev.events |= EPOLLIN;
    }
    if (iter->second->register_events & EventTypes::kWrite)
    {
        ev.events |= EPOLLOUT;
    }
    ev.data.fd = fd;
    epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
}

void EpollMultiplex::SetTimeoutMs(int timeout_ms)
{
    timeout_ms_ = timeout_ms;
}

int EpollMultiplex::ExecuteOnce()
{
    int nfds = epoll_wait(epoll_fd_, &epoll_events_[0], static_cast<int>(epoll_events_.size()), timeout_ms_);
    if (nfds > 0)
    {
        for (int i = 0; i < nfds; ++i)
        {
            int triggered_events = 0;
            if (epoll_events_[i].events & EPOLLIN)
            {
                triggered_events |= EventTypes::kRead;
            }
            if (epoll_events_[i].events & EPOLLOUT)
            {
                triggered_events |= EventTypes::kWrite;
            }

            auto handler_iter = handlers_.find(epoll_events_[i].data.fd);
            if (handlers_.end() != handler_iter)
            {
                handler_iter->second->callback(handler_iter->first, triggered_events);
            }
        }
    }
    else if (nfds < 0)
    {
        int err = lccl::skt::GetLastErrorCode();
        if (EINTR == err)
        {
            nfds = 0;
        }
    }

    return nfds;
}

LCCL_EVENT_NAMESPACE_END
LCCL_NAMESPACE_END

#endif
