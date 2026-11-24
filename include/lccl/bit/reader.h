#ifndef LCCL_INCLUDE_LCCL_BIT_READER_H_
#define LCCL_INCLUDE_LCCL_BIT_READER_H_

#include "lccl/bit/defines.h"

LCCL_NAMESPACE_BEGIN
LCCL_BIT_NAMESPACE_BEGIN

class BitReader
{
public:
    struct Pointer
    {
        const uint8_t *ptr = nullptr;
        size_t bits = 0;
    };

public:
    BitReader(const uint8_t *data, size_t size, ByteOrders byte_order = ByteOrders::kBe) :
        data_{ data },
        end_{ data + size },
        byte_order_{ byte_order },
        pointer_{ data, 0 }
    {

    }

    virtual ~BitReader() = default;

    const Pointer &GetPointer() const
    {
        return pointer_;
    }

    void SetPointer(const Pointer &pointer)
    {
        pointer_ = pointer;
    }

    size_t Size() const
    {
        return (0 == pointer_.bits) ? (pointer_.ptr - data_) : (pointer_.ptr - data_ + 1);
    }

    bool Check() const
    {
        if (((pointer_.ptr >= data_) && (pointer_.ptr < end_)) ||
            ((pointer_.ptr == end_) && (0 == pointer_.bits)))
        {
            return true;
        }

        return false;
    }

    void Skip(size_t bytes, size_t bits)
    {
        pointer_.ptr += bytes;
        pointer_.bits += bits;
        pointer_.ptr += (pointer_.bits >> 3);
        pointer_.bits &= 0b111;
    }

    uint64_t Read1Bit()
    {
        uint64_t val = (pointer_.ptr[0] >> (7 - pointer_.bits)) & 0b1;
        Skip(0, 1);
        return val;
    }

    uint64_t ReadBits(size_t bits)
    {
        uint64_t val = 0;
        if (0 != pointer_.bits)
        {
            val = pointer_.ptr[0] & kBitMasks[pointer_.bits];
            pointer_.bits += bits;
            if (pointer_.bits < 8)
            {
                val >>= (8 - pointer_.bits);
                return val;
            }

            bits = pointer_.bits - 8;
            ++pointer_.ptr;
            pointer_.bits = 0;
        }

        for (size_t bytes = bits >> 3; bytes > 0; --bytes)
        {
            val = (val << 8) + pointer_.ptr[0];
            ++pointer_.ptr;
        }

        bits = bits & 0b111;
        if (bits > 0)
        {
            val = (val << bits) + (pointer_.ptr[0] >> (8 - bits));
            pointer_.bits = bits;
        }

        return val;
    }

    uint64_t ReadUe()
    {
        size_t leading_zero_bits = 0;
        while (0 == Read1Bit())
        {
            ++leading_zero_bits;
            if (!Check())
            {
                return 0;
            }
        }

        return (1ULL << leading_zero_bits) + ReadBits(leading_zero_bits) - 1;
    }

    int64_t ReadSe()
    {
        uint64_t val = ReadUe();
        if (0 == (val & 0b1))
        {
            return -static_cast<int64_t>(val >> 1);
        }

        return static_cast<int64_t>(val >> 1) + 1;
    }

    void ReadAlign()
    {
        if (0 != pointer_.bits)
        {
            ++pointer_.ptr;
            pointer_.bits = 0;
        }
    }

    uint64_t Read1Byte()
    {
        if (0 == pointer_.bits)
        {
            ++pointer_.ptr;
            return *(pointer_.ptr - 1);
        }

        return ReadBits(8);
    }

    uint64_t Read2Bytes()
    {
        uint64_t low_val = 0;
        uint64_t high_val = 0;
        switch (byte_order_)
        {
        case ByteOrders::kBe:
            high_val = Read1Byte();
            low_val = Read1Byte();
            break;

        case ByteOrders::kLe:
            low_val = Read1Byte();
            high_val = Read1Byte();
            break;

        default:
            break;
        }

        return (high_val << 8) + low_val;
    }

    uint64_t Read4Bytes()
    {
        uint64_t low_val = 0;
        uint64_t high_val = 0;
        switch (byte_order_)
        {
        case ByteOrders::kBe:
            high_val = Read2Bytes();
            low_val = Read2Bytes();
            break;

        case ByteOrders::kLe:
            low_val = Read2Bytes();
            high_val = Read2Bytes();
            break;

        default:
            break;
        }

        return (high_val << 16) + low_val;
    }

    uint64_t Read8Bytes()
    {
        uint64_t low_val = 0;
        uint64_t high_val = 0;
        switch (byte_order_)
        {
        case ByteOrders::kBe:
            high_val = Read4Bytes();
            low_val = Read4Bytes();
            break;

        case ByteOrders::kLe:
            low_val = Read4Bytes();
            high_val = Read4Bytes();
            break;

        default:
            break;
        }

        return (high_val << 32) + low_val;
    }

    void ReadByteStream(uint8_t *buf, size_t size)
    {
        if (0 == pointer_.bits)
        {
            memcpy(buf, pointer_.ptr, size);
            pointer_.ptr += size;
            return;
        }

        for (; size > 0; --size)
        {
            *buf++ = static_cast<uint8_t>(ReadBits(8));
        }
    }

private:
    const uint8_t *data_;
    const uint8_t *end_;
    ByteOrders byte_order_;

    Pointer pointer_;
};

LCCL_BIT_NAMESPACE_END
LCCL_NAMESPACE_END

#endif // !LCCL_INCLUDE_LCCL_BIT_READER_H_
