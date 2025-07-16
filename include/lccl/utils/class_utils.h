#ifndef LCCL_INCLUDE_LCCL_UTILS_CLASS_UTILS_H_
#define LCCL_INCLUDE_LCCL_UTILS_CLASS_UTILS_H_

#include <utility>
#include "lccl.h"

LCCL_NAMESPACE_BEGIN

template <typename T, typename... Args>
std::shared_ptr<T> MakeSharedProtected(Args &&...args)
{
    struct Enabler : public T
    {
        Enabler(Args &&... args) : T(std::forward<Args>(args)...) {}
    };
    return std::make_shared<Enabler>(std::forward<Args>(args)...);
}

template <typename T, typename Allocator, typename... Args>
std::shared_ptr<T> AllocateSharedProtected(const Allocator &alloc, Args &&...args)
{
    struct Enabler : public T
    {
        Enabler(Args &&... args) : T(std::forward<Args>(args)...) {}
    };
    return std::allocate_shared<Enabler>(alloc, std::forward<Args>(args)...);
}

LCCL_NAMESPACE_END

#endif // !LCCL_INCLUDE_LCCL_UTILS_CLASS_UTILS_H_
