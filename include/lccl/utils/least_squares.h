#ifndef LCCL_INCLUDE_LCCL_UTILS_LEAST_SQUARES_H_
#define LCCL_INCLUDE_LCCL_UTILS_LEAST_SQUARES_H_

#include <cstdint>
#include "ring_buffer.h"

LCCL_NAMESPACE_BEGIN

// 最小二乘法实现
class LeastSquares
{
public:
    explicit LeastSquares(size_t power) :
        x_(power),
        y_(power)
    {
        Reset();
    }

    // 尾部添加数据
    void PushBack(int64_t x, int64_t y)
    {
        while (!x_.PushBack(x))
        {
            PopFront();
        }
        y_.PushBack(y);

        const size_t n = x_.Size();
        if (1 == n)
        {
            x0_ = x;
            y0_ = y;

            mean_x_ = 0;
            mean_y_ = 0;
            sxx_ = 0;
            sxy_ = 0;
            return;
        }

        const long double vx = Diff(x, x0_);
        const long double vy = Diff(y, y0_);

        const long double dx = vx - mean_x_;
        const long double dy = vy - mean_y_;

        mean_x_ += dx / n;
        mean_y_ += dy / n;

        sxx_ += dx * (vx - mean_x_);
        sxy_ += dx * (vy - mean_y_);
    }

    // 头部删除数据
    bool PopFront()
    {
        if (x_.Empty())
        {
            return false;
        }

        const long double vx = Diff(x_.Front(), x0_);
        const long double vy = Diff(y_.Front(), y0_);

        x_.PopFront();
        y_.PopFront();

        const size_t n = x_.Size();
        if (0 == n)
        {
            Reset();
            return true;
        }

        if (1 == n)
        {
            x0_ = x_.Front();
            y0_ = y_.Front();

            mean_x_ = 0;
            mean_y_ = 0;
            sxx_ = 0;
            sxy_ = 0;
            return true;
        }

        const long double dx = vx - mean_x_;
        const long double dy = vy - mean_y_;

        mean_x_ -= dx / n;
        mean_y_ -= dy / n;

        sxx_ -= dx * (vx - mean_x_);
        sxy_ -= dx * (vy - mean_y_);
        return true;
    }

    // 获取斜率
    long double Slope() const
    {
        return sxy_ / sxx_;
    }

    // 获取截距
    long double Intercept() const
    {
        const long double k = Slope();
        return static_cast<long double>(y0_) + mean_y_ - k * (static_cast<long double>(x0_) + mean_x_);
    }

    // 预测 y，使用相对坐标降低精度损失
    long double Predict(int64_t x) const
    {
        const long double k = Slope();
        return static_cast<long double>(y0_) + mean_y_ + k * (Diff(x, x0_) - mean_x_);
    }

    size_t Size() const
    {
        return x_.Size();
    }

    bool Empty() const
    {
        return x_.Empty();
    }

    void Clear()
    {
        x_.Clear();
        y_.Clear();
        Reset();
    }

    const RingBuffer<int64_t> &GetX() const
    {
        return x_;
    }

    const RingBuffer<int64_t> &GetY() const
    {
        return y_;
    }

private:
    void Reset()
    {
        x0_ = 0;
        y0_ = 0;
        mean_x_ = 0;
        mean_y_ = 0;
        sxx_ = 0;
        sxy_ = 0;
    }

    // 安全计算两个int64_t的差值，避免有符号整数溢出
    static long double Diff(int64_t a, int64_t b)
    {
        if (a >= b)
        {
            return static_cast<long double>(static_cast<uint64_t>(a) - static_cast<uint64_t>(b));
        }

        return -static_cast<long double>(static_cast<uint64_t>(b) - static_cast<uint64_t>(a));
    }

private:
    RingBuffer<int64_t> x_;
    RingBuffer<int64_t> y_;

    int64_t x0_;
    int64_t y0_;

    long double mean_x_;
    long double mean_y_;

    long double sxx_;
    long double sxy_;
};

LCCL_NAMESPACE_END

#endif // !LCCL_INCLUDE_LCCL_UTILS_LEAST_SQUARES_H_
