#ifndef LCCL_INCLUDE_LCCL_UTILS_RING_BUFFER_H_
#define LCCL_INCLUDE_LCCL_UTILS_RING_BUFFER_H_

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <vector>
#include "lccl.h"

LCCL_BEGIN_NAMESPACE

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
    RingBuffer(size_t power) :
        size_(static_cast<size_t>(2) << power),
        size_mask_(size_ - 1),
        buffer_(size_),
        write_index_(0),
        read_index_(0)
    {

    }

    virtual ~RingBuffer()
    {

    }

    void Swap(RingBuffer &rhs)
    {
        std::swap(size_, rhs.size_);
        std::swap(size_mask_, rhs.size_mask_);
        std::swap(buffer_, rhs.buffer_);
        std::swap(write_index_, rhs.write_index_);
        std::swap(read_index_, rhs.read_index_);
    }

    bool Push(const T &item)
    {
        size_t next_write_index = (write_index_ + 1) & size_mask_;

        // 判断缓冲区是否已满
        if (next_write_index == read_index_)
        {
            return false;
        }

        buffer_[write_index_] = item;
        write_index_ = next_write_index;
        return true;
    }

    bool Pop(T &item)
    {
        // 判断缓冲区是否是空的
        if (read_index_ == write_index_)
        {
            return false;
        }

        item = buffer_[read_index_];
        read_index_ = (read_index_ + 1) & size_mask_;
        return true;
    }

    void Clear()
    {
        read_index_ = write_index_;
    }

    bool Empty() const
    {
        return (read_index_ == write_index_);
    }

    T &At(size_t index)
    {
        return buffer_[(read_index_ + index) & size_mask_];
    }

    size_t Size() const
    {
        if (write_index_ >= read_index_)
        {
            return write_index_ - read_index_;
        }

        return read_index_ + size_ - write_index_;
    }

private:
    size_t size_;
    size_t size_mask_;

    std::vector<T> buffer_;
    size_t write_index_;
    size_t read_index_;
};

// 单生产者单消费者无锁版本
// 输入冪次，size是2的冪
template <typename T>
class LockFreeRingBuffer
{
public:
    LockFreeRingBuffer(size_t power) :
        size_(static_cast<size_t>(1) << power),
        size_mask_(size_ -1),
        buffer_(size_)
    {
        write_index_.store(0, std::memory_order_relaxed);
        read_index_.store(0, std::memory_order_relaxed);
    }

    virtual ~LockFreeRingBuffer()
    {

    }

    bool Push(const T &item)
    {
        size_t write_index = write_index_.load(std::memory_order_relaxed);
        size_t next_write_index = (write_index + 1) & size_mask_;

        // 判断缓冲区是否已满
        if (next_write_index == read_index_.load(std::memory_order_relaxed))
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
        if (read_index == write_index_.load(std::memory_order_relaxed))
        {
            return false;
        }

        item = buffer_[read_index];
        read_index_.store((read_index + 1) & size_mask_, std::memory_order_release);
        return true;
    }

    // 清空环形缓冲区，只能在pop端调用
    void PopClear()
    {
        size_t write_index = write_index_.load(std::memory_order_relaxed);
        read_index_.store(write_index, std::memory_order_release);
    }

    bool Empty() const
    {
        return (read_index_.load(std::memory_order_relaxed) ==
            write_index_.load(std::memory_order_relaxed));
    }

    bool Full() const
    {
        return ((write_index_.load(std::memory_order_relaxed) + 1) & size_mask_) ==
            read_index_.load(std::memory_order_relaxed);
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
