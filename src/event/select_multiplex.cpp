#include <thread>
#include "lccl/socket.h"
#include "event/select_multiplex.h"

LCCL_NAMESPACE_BEGIN
LCCL_EVENT_NAMESPACE_BEGIN

SelectMultiplex::SelectMultiplex(int timeout_ms) :
    timeout_ms_(timeout_ms),
    max_fd_(-1)
{

}

SelectMultiplex::~SelectMultiplex()
{

}

void SelectMultiplex::RegisterHandler(int fd, int register_events, HandlerCallback callback)
{
    std::shared_ptr<Handler> handler = std::make_shared<Handler>();
    handler->callback = callback;
    handler->register_events = register_events;

    RemoveHandler(fd);
    handlers_.emplace(fd, handler);

    if (fd > max_fd_)
    {
        max_fd_ = fd;
    }
}

void SelectMultiplex::RemoveHandler(int fd)
{
    auto iter = handlers_.find(fd);
    if (handlers_.end() == iter)
    {
        return;
    }

    handlers_.erase(iter);
    if (!handlers_.empty())
    {
        max_fd_ = handlers_.rbegin()->first;
    }
    else
    {
        max_fd_ = -1;
    }
}

void SelectMultiplex::EnableEvent(int fd, int events)
{
    auto iter = handlers_.find(fd);
    if (handlers_.end() == iter)
    {
        return;
    }

    iter->second->register_events |= events;
}

void SelectMultiplex::DisableEvent(int fd, int events)
{
    auto iter = handlers_.find(fd);
    if (handlers_.end() == iter)
    {
        return;
    }

    iter->second->register_events &= ~events;
}

void SelectMultiplex::SetTimeoutMs(int timeout_ms)
{
    timeout_ms_ = timeout_ms;
}

int SelectMultiplex::ExecuteOnce()
{
    if (max_fd_ < 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms_));
        return 0;
    }

    fd_set rfds, wfds;
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);

    for (auto &&iter : handlers_)
    {
        if (iter.second->register_events & EventTypes::kRead)
        {
            FD_SET(iter.first, &rfds);
        }

        if (iter.second->register_events & EventTypes::kWrite)
        {
            FD_SET(iter.first, &wfds);
        }
    }

    timeval timeout_tv = { 0, 1000 * timeout_ms_ };
    int select_ret = select(max_fd_ + 1, &rfds, &wfds, nullptr, &timeout_tv);
    if (select_ret > 0)
    {
        for (auto &&iter : handlers_)
        {
            int triggered_events = 0;
            if (FD_ISSET(iter.first, &rfds))
            {
                triggered_events |= EventTypes::kRead;
            }

            if (FD_ISSET(iter.first, &wfds))
            {
                triggered_events |= EventTypes::kWrite;
            }

            if (triggered_events > 0)
            {
                iter.second->callback(iter.first, triggered_events);
            }
        }
    }
    else if (select_ret < 0)
    {
        int err = lccl::skt::GetLastErrorCode();

#if defined(_WIN32)
        if (WSAEINTR == err)
#else
        if (EINTR == err)
#endif
        {
            select_ret = 0;
        }
    }

    return select_ret;
}

LCCL_EVENT_NAMESPACE_END
LCCL_NAMESPACE_END
