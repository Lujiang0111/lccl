#ifndef LCCL_INTERNAL_EVENT_SELECT_MULTIPLEX_H_
#define LCCL_INTERNAL_EVENT_SELECT_MULTIPLEX_H_

#include <map>
#include "lccl/event.h"

LCCL_NAMESPACE_BEGIN
LCCL_EVENT_NAMESPACE_BEGIN

class SelectMultiplex : public IMultiplex
{
public:
    SelectMultiplex() = delete;
    SelectMultiplex(const SelectMultiplex &) = delete;
    SelectMultiplex &operator=(const SelectMultiplex &) = delete;

    explicit SelectMultiplex(int timeout_ms);
    virtual ~SelectMultiplex();

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
    int max_fd_;
    std::map<int, std::shared_ptr<Handler>> handlers_;
};

LCCL_EVENT_NAMESPACE_END
LCCL_NAMESPACE_END


#endif // !LCCL_INTERNAL_EVENT_SELECT_MULTIPLEX_H_
