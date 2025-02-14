#ifndef LCCL_INCLUDE_LCCL_UTILS_RING_BUFFER_H_
#define LCCL_INCLUDE_LCCL_UTILS_RING_BUFFER_H_

#include <cstddef>
#include <atomic>
#include <vector>
#include "lccl.h"

LCCL_BEGIN_NAMESPACE

// 单生产者单消费者无锁环形缓冲区
// 输入冪次，size是2的冪
template <typename T>
class RingBuffer
{
public:
    RingBuffer(size_t power) :
        size_(static_cast<size_t>(2) << power),
        size_mask_(size_ -1),
        buffer_(size_)
    {
        write_index_.store(0, std::memory_order_relaxed);
        read_index_.store(0, std::memory_order_relaxed);
    }

    bool Push(const T &item)
    {
        size_t write_index = write_index_.load(std::memory_order_relaxed);
        size_t next_write_index = (write_index + 1) & size_mask_;

        // 判断缓冲区是否已满
        if (next_write_index == read_index_.load(std::memory_order_acquire))
        {
            return false;
        }

        buffer_[write_index] = item;
        write_index_.store(next_write_index, std::memory_order_release);
        return true;
    }

    bool Pop(T &item)
    {
        size_t read_index = read_index_.load(std::memory_order_relaxed);

        // 判断缓冲区是否是空的
        if (read_index == write_index_.load(std::memory_order_acquire))
        {
            return false;
        }

        item = buffer_[read_index];
        read_index_.store((read_index + 1) & size_mask_, std::memory_order_release);
        return true;
    }

    bool Empty()
    {
        return (read_index_.load(std::memory_order_acquire) == write_index_.load(std::memory_order_acquire));
    }

private:
    size_t size_;
    size_t size_mask_;

    std::vector<T> buffer_;
    std::atomic<size_t> write_index_;
    std::atomic<size_t> read_index_;
};

LCCL_END_NAMESPACE

#endif // !LCCL_INCLUDE_LCCL_UTILS_RING_BUFFER_H_
