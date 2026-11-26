#ifndef LCCL_INCLUDE_LCCL_UTILS_RING_BUFFER_H_
#define LCCL_INCLUDE_LCCL_UTILS_RING_BUFFER_H_

#include <atomic>
#include <cstddef>
#include <vector>
#include <utility>
#include "lccl.h"

LCCL_NAMESPACE_BEGIN

/**
* 环形缓冲区：两种版本
* 线程不安全
* 单生产者单消费者无锁
*/

// 线程不安全版本
// 输入冪次，size是2的冪
template <typename T>
class RingBuffer
{
public:
    using container_type = std::vector<T>;
    using reference = typename container_type::reference;
    using const_reference = typename container_type::const_reference;

public:
    explicit RingBuffer(size_t power) :
        buffer_(static_cast<size_t>(1) << power),
        mask_(buffer_.size() - 1),
        head_(0),
        size_(0)
    {

    }

    void Swap(RingBuffer &rhs)
    {
        buffer_.swap(rhs.buffer_);
        std::swap(mask_, rhs.mask_);
        std::swap(head_, rhs.head_);
        std::swap(size_, rhs.size_);
    }

    bool PushBack(const T &value)
    {
        if (Full())
        {
            return false;
        }

        buffer_[(head_ + size_) & mask_] = value;
        ++size_;
        return true;
    }

    bool PushBack(T &&value)
    {
        if (Full())
        {
            return false;
        }

        buffer_[(head_ + size_) & mask_] = std::move(value);
        ++size_;
        return true;
    }

    bool PopFront(bool reset = false)
    {
        if (Empty())
        {
            return false;
        }

        if (reset)
        {
            buffer_[head_] = T{};
        }

        head_ = (head_ + 1) & mask_;
        --size_;
        return true;
    }

    template <typename Callback>
    bool PopBulk(Callback &&callback, bool reset = false)
    {
        if (Empty())
        {
            return false;
        }

        for (size_t i = 0; i < size_; ++i)
        {
            reference value = buffer_[(head_ + i) & mask_];
            callback(value);

            if (reset)
            {
                value = T{};
            }
        }

        head_ = 0;
        size_ = 0;
        return true;
    }

    reference Front()
    {
        return buffer_[head_];
    }

    const_reference Front() const
    {
        return buffer_[head_];
    }

    reference Back()
    {
        return buffer_[(head_ + size_ - 1) & mask_];
    }

    const_reference Back() const
    {
        return buffer_[(head_ + size_ - 1) & mask_];
    }

    bool Empty() const
    {
        return size_ == 0;
    }

    bool Full() const
    {
        return size_ == buffer_.size();
    }

    size_t Size() const
    {
        return size_;
    }

    size_t Capacity() const
    {
        return buffer_.size();
    }

    void Clear(bool reset = false)
    {
        if (reset)
        {
            for (size_t i = 0; i < size_; ++i)
            {
                buffer_[(head_ + i) & mask_] = T{};
            }
        }

        head_ = 0;
        size_ = 0;
    }

private:
    std::vector<T> buffer_;
    size_t mask_;
    size_t head_;
    size_t size_;
};

// 单生产者单消费者无锁版本
// 输入冪次，size是2的冪
template <typename T>
class LockFreeRingBuffer
{
public:
    explicit LockFreeRingBuffer(size_t power) :
        buffer_(static_cast<size_t>(1) << power),
        mask_(buffer_.size() - 1),
        head_(0),
        cached_tail_(0),
        tail_(0),
        cached_head_(0)
    {

    }

    LockFreeRingBuffer(const LockFreeRingBuffer &) = delete;
    LockFreeRingBuffer &operator=(const LockFreeRingBuffer &) = delete;

    // 仅生产者线程调用
    bool Push(const T &value)
    {
        return PushImpl(value);
    }

    // 支持移动语义
    bool Push(T &&value)
    {
        return PushImpl(std::move(value));
    }

    // 仅消费者线程调用
    bool Pop(T &value, bool reset = false)
    {
        const size_t tail = tail_.load(std::memory_order_relaxed);

        if (tail == cached_head_)
        {
            cached_head_ = head_.load(std::memory_order_acquire);

            if (tail == cached_head_)
            {
                return false;
            }
        }

        T &item = buffer_[tail & mask_];
        value = std::move(item);
        if (reset)
        {
            item = T{};
        }

        tail_.store(tail + 1, std::memory_order_release);
        return true;
    }

    // 仅消费者线程调用
    template <typename Callback>
    bool PopBulk(Callback &&callback, bool reset = false)
    {
        const size_t tail = tail_.load(std::memory_order_relaxed);
        const size_t head = head_.load(std::memory_order_acquire);
        cached_head_ = head;

        if (head == tail)
        {
            return false;
        }

        for (size_t pos = tail; pos != head; ++pos)
        {
            T &item = buffer_[pos & mask_];
            callback(item);
            if (reset)
            {
                item = T{};
            }
        }

        tail_.store(head, std::memory_order_release);
        return true;
    }

    // 仅生产者线程调用
    bool Full() const
    {
        const size_t head = head_.load(std::memory_order_relaxed);
        const size_t tail = tail_.load(std::memory_order_acquire);
        return head - tail == buffer_.size();
    }

    // 仅消费者线程调用
    bool Empty() const
    {
        const size_t tail = tail_.load(std::memory_order_relaxed);
        const size_t head = head_.load(std::memory_order_acquire);
        return head == tail;
    }

private:
    template <typename U>
    bool PushImpl(U &&value)
    {
        const size_t head = head_.load(std::memory_order_relaxed);

        if (head - cached_tail_ == buffer_.size())
        {
            cached_tail_ = tail_.load(std::memory_order_acquire);
            if (head - cached_tail_ == buffer_.size())
            {
                return false;
            }
        }

        buffer_[head & mask_] = std::forward<U>(value);
        head_.store(head + 1, std::memory_order_release);
        return true;
    }

private:
    std::vector<T> buffer_;
    const size_t mask_;

    // 生产者线程修改
    alignas(64) std::atomic<size_t> head_;
    size_t cached_tail_;

    // 消费者线程修改
    alignas(64) std::atomic<size_t> tail_;
    size_t cached_head_;
};

LCCL_NAMESPACE_END

#endif // !LCCL_INCLUDE_LCCL_UTILS_RING_BUFFER_H_
