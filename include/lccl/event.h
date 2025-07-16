#ifndef LCCL_INCLUDE_LCCL_EVENT_H_
#define LCCL_INCLUDE_LCCL_EVENT_H_

#include <functional>
#include "lccl.h"

#define LCCL_EVENT_NAMESPACE_BEGIN namespace evt {
#define LCCL_EVENT_NAMESPACE_END }

LCCL_NAMESPACE_BEGIN
LCCL_EVENT_NAMESPACE_BEGIN

enum class MultiplexTypes
{
    kAuto = 0,
    kEpoll,
    kSelect,
};

enum EventTypes : int
{
    kRead = 0x01,
    kWrite = 0x02,
};

class IMultiplex
{
public:
    virtual ~IMultiplex() = default;

    using HandlerCallback = std::function<void(int fd, int triggered_events)>;
    virtual void RegisterHandler(int fd, int register_events, HandlerCallback callback) = 0;
    virtual void RemoveHandler(int fd) = 0;

    virtual void EnableEvent(int fd, int events) = 0;
    virtual void DisableEvent(int fd, int events) = 0;

    virtual void SetTimeoutMs(int timeout_ms) = 0;

    virtual int ExecuteOnce() = 0;
};

LCCL_API std::shared_ptr<IMultiplex> CreateMultiplex(MultiplexTypes type, int timeout_ms);


LCCL_EVENT_NAMESPACE_END
LCCL_NAMESPACE_END

#endif // !LCCL_INCLUDE_LCCL_EVENT_H_
