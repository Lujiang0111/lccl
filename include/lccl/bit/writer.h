#ifndef LCCL_INCLUDE_LCCL_BIT_WRITER_H_
#define LCCL_INCLUDE_LCCL_BIT_WRITER_H_

#include "lccl/bit/defines.h"

LCCL_NAMESPACE_BEGIN
LCCL_BIT_NAMESPACE_BEGIN

class Writer
{
public:
    // data为nullptr时不写入，只计算长度
    Writer(uint8_t *data, ByteOrders byte_order = ByteOrders::kBe) :
        data_(data),
        byte_order_(byte_order),
        curr_save_point_(data_)
    {

    }

    virtual ~Writer()
    {

    }

    // bit
    template <typename T>
    void WriteBits(T val, size_t bits)
    {
        if (!data_)
        {
            SkipBits(bits);
            return;
        }

        if (0 != curr_save_point_.bit_pos)
        {
            if (curr_save_point_.bit_pos + bits < 8)
            {
                uint8_t mask = kBitNegMasks[curr_save_point_.bit_pos] | kBitMasks[curr_save_point_.bit_pos + bits];
                *curr_save_point_.ptr = (*curr_save_point_.ptr & mask) |
                    ((val & kBitMasks[8 - bits]) << (8 - curr_save_point_.bit_pos - bits));
                curr_save_point_.bit_pos += bits;
                return;
            }

            *curr_save_point_.ptr = (*curr_save_point_.ptr & kBitNegMasks[curr_save_point_.bit_pos]) |
                ((val >> (bits - (8 - curr_save_point_.bit_pos))) & kBitMasks[curr_save_point_.bit_pos]);

            bits -= (8 - curr_save_point_.bit_pos);
            curr_save_point_.bit_pos = 0;
            ++curr_save_point_.ptr;
        }

        size_t bytes = bits >> 3;
        bits &= 0b111;
        int64_t bytes_val = val >> bits;

        curr_save_point_.ptr += bytes;
        for (uint8_t *ptr = curr_save_point_.ptr; bytes > 0; --bytes)
        {
            *--ptr = bytes_val & 0xFF;
            bytes_val >>= 8;
        }

        if (bits > 0)
        {
            *curr_save_point_.ptr = (*curr_save_point_.ptr & kBitMasks[bits]) | ((val << (8 - bits)) & 0xFF);
            curr_save_point_.bit_pos = bits;
        }
    }

    template <typename T>
    void WriteUe(T val)
    {
        size_t bits = 0;
        for (T tmp = val + 1; tmp > 0; tmp >>= 1)
        {
            ++bits;
        }
        bits += (bits - 1);

        WriteBits(val + 1, bits);
    }

    template <typename T>
    void WriteSe(T val)
    {
        WriteUe((val > 0) ? (2 * val - 1) : (-val * 2));
    }

    void WriteAlign()
    {
        if (0 != curr_save_point_.bit_pos)
        {
            if (data_)
            {
                *curr_save_point_.ptr &= kBitNegMasks[curr_save_point_.bit_pos];
            }

            ++curr_save_point_.ptr;
            curr_save_point_.bit_pos = 0;
        }
    }

    // byte
    template <typename T>
    void WriteByte(T val)
    {
        if (!data_)
        {
            ++curr_save_point_.ptr;
            return;
        }

        if (0 == curr_save_point_.bit_pos)
        {
            *curr_save_point_.ptr = val & 0xFF;
            ++curr_save_point_.ptr;
        }
        else
        {
            WriteBits(val, 8);
        }
    }

    template <typename T>
    void Write2Bytes(T val)
    {
        if (!data_)
        {
            curr_save_point_.ptr += 2;
            return;
        }

        T low_val = val;
        T high_val = val >> 8;
        switch (byte_order_)
        {
        case ByteOrders::kBe:
            WriteByte(high_val);
            WriteByte(low_val);
            break;

        case ByteOrders::kLe:
            WriteByte(low_val);
            WriteByte(high_val);
            break;

        default:
            break;
        }
    }

    template <typename T>
    void Write4Bytes(T val)
    {
        if (!data_)
        {
            curr_save_point_.ptr += 4;
            return;
        }

        T low_val = val;
        T high_val = val >> 16;
        switch (byte_order_)
        {
        case ByteOrders::kBe:
            Write2Bytes(high_val);
            Write2Bytes(low_val);
            break;

        case ByteOrders::kLe:
            Write2Bytes(low_val);
            Write2Bytes(high_val);
            break;

        default:
            break;
        }
    }

    template <typename T>
    void Write8Bytes(T val)
    {
        if (!data_)
        {
            curr_save_point_.ptr += 8;
            return;
        }

        T low_val = val;
        T high_val = val >> 32;
        switch (byte_order_)
        {
        case ByteOrders::kBe:
            Write4Bytes(high_val);
            Write4Bytes(low_val);
            break;

        case ByteOrders::kLe:
            Write4Bytes(low_val);
            Write4Bytes(high_val);
            break;

        default:
            break;
        }
    }

    // byte stream
    void WriteByteStream(const uint8_t *data, size_t size)
    {
        if (!data_)
        {
            curr_save_point_.ptr += size;
            return;
        }

        if (0 == curr_save_point_.bit_pos)
        {
            memcpy(curr_save_point_.ptr, data, size);
            curr_save_point_.ptr += size;
        }
        else
        {
            for (; size > 0; --size)
            {
                WriteByte(*data);
                ++data;
            }
            return;
        }
    }

    void FillBytes(uint8_t val, size_t size)
    {
        if (!data_)
        {
            curr_save_point_.ptr += size;
            return;
        }

        if (0 == curr_save_point_.bit_pos)
        {
            memset(curr_save_point_.ptr, val, size);
            curr_save_point_.ptr += size;
        }
        else
        {
            for (; size > 0; --size)
            {
                WriteByte(val);
            }
            return;
        }
    }

    // skip
    void SkipBits(size_t bits)
    {
        curr_save_point_.bit_pos += bits;
        curr_save_point_.ptr += (curr_save_point_.bit_pos >> 3);
        curr_save_point_.bit_pos &= 0b111;
    }

    void SkipBytes(size_t bytes)
    {
        curr_save_point_.ptr += bytes;
    }

    // save/load
    const SavePoint &Save() const
    {
        return curr_save_point_;
    }

    void Load(const SavePoint &save_point)
    {
        curr_save_point_ = save_point;
    }

    uint8_t *CurrPtr() const
    {
        return curr_save_point_.ptr;
    }

    size_t CurrSize() const
    {
        size_t offset = curr_save_point_.ptr - data_;
        return (0 == curr_save_point_.bit_pos) ? (offset) : (offset + 1);
    }

private:
    uint8_t *data_;
    ByteOrders byte_order_;
    SavePoint curr_save_point_;
};

LCCL_BIT_NAMESPACE_END
LCCL_NAMESPACE_END

#endif // !LCCL_INCLUDE_LCCL_BIT_WRITER_H_
