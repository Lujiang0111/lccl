#include "event/epoll_multiplex.h"
#include "event/select_multiplex.h"

LCCL_BEGIN_NAMESPACE
LCCL_EVENT_BEGIN_NAMESPACE

std::shared_ptr<IMultiplex> CreateMultiplex(MultiplexTypes type, int timeout_ms)
{
    if (MultiplexTypes::kAuto == type)
    {
#if defined(_MSC_VER)
        type = MultiplexTypes::kSelect;
#else
        type = MultiplexTypes::kEpoll;
#endif
    }

    std::shared_ptr<IMultiplex> h = nullptr;
    switch (type)
    {
    case MultiplexTypes::kEpoll:
#if defined(_MSC_VER)
#else
        h = std::make_shared<EpollMultiplex>(timeout_ms);
#endif
        break;
    case MultiplexTypes::kSelect:
        h = std::make_shared<SelectMultiplex>(timeout_ms);
        break;
    default:
        break;
    }

    return h;
}

LCCL_EVENT_END_NAMESPACE
LCCL_END_NAMESPACE
