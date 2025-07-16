#include <deque>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include "lccl/utils/ring_buffer.h"
#include "test_ring_buffer.h"

struct ItemCtx
{
    int test_no = 0;
};

struct TestRingBufferCtx
{
    lccl::RingBuffer<std::shared_ptr<ItemCtx>> ring_buffer{ 10 };
    std::mutex ring_buffer_mutex;

    lccl::LockFreeRingBuffer<std::shared_ptr<ItemCtx>> lock_free_ring_buffer{ 10 };
};

extern bool app_running;

void RingBufferThread(TestRingBufferCtx *test_ctx)
{
    std::deque<std::shared_ptr<ItemCtx>> out;

    while (app_running)
    {
        {
            std::lock_guard<std::mutex> lock(test_ctx->ring_buffer_mutex);
            test_ctx->ring_buffer.PopBulkAndSetNull([&out](std::shared_ptr<ItemCtx> &item)
                {
                    out.push_back(item);
                });
        }

        while (!out.empty())
        {
            std::cout << "ring buffer: " << out.front()->test_no << std::endl;
            out.pop_front();
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void LockFreeRingBufferThread(TestRingBufferCtx *test_ctx)
{
    std::deque<std::shared_ptr<ItemCtx>> out;

    while (app_running)
    {
        {
            test_ctx->lock_free_ring_buffer.PopBulkAndSetNull([&out](std::shared_ptr<ItemCtx> &item)
                {
                    out.push_back(item);
                });
        }

        while (!out.empty())
        {
            std::cout << "lock free ring buffer: " << out.front()->test_no << std::endl;
            out.pop_front();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void TestRingBuffer(int argc, char **argv)
{
    TestRingBufferCtx test_ctx;
    
    std::thread ring_buffer_thread(RingBufferThread, &test_ctx);
    std::thread lock_free_ring_buffer_thread(LockFreeRingBufferThread, &test_ctx);
    
    int test_no = 0;
    while (app_running)
    {
        ++test_no;
        std::shared_ptr<ItemCtx> item = std::make_shared<ItemCtx>();
        item->test_no = test_no;

        {
            std::lock_guard<std::mutex> lock(test_ctx.ring_buffer_mutex);
            test_ctx.ring_buffer.Push(item);
        }

        test_ctx.lock_free_ring_buffer.Push(item);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (ring_buffer_thread.joinable())
    {
        ring_buffer_thread.join();
    }

    if (lock_free_ring_buffer_thread.joinable())
    {
        lock_free_ring_buffer_thread.join();
    }
}
