#ifndef LCCL_INCLUDE_LCCL_BIT_READER_H_
#define LCCL_INCLUDE_LCCL_BIT_READER_H_

#include "lccl/bit/defines.h"

LCCL_BEGIN_NAMESPACE
LCCL_BIT_BEGIN_NAMESPACE

class Reader
{
public:
    Reader(const uint8_t *data, ByteOrders byte_order = ByteOrders::kBe) :
        data_(const_cast<uint8_t *>(data)),
        byte_order_(byte_order),
        curr_save_point_(data_)
    {

    }

    virtual ~Reader()
    {

    }

    // bit
    int64_t ReadBit()
    {
        int64_t val = *curr_save_point_.ptr & kBitPoints[curr_save_point_.bit_pos];
        SkipBits(1);
        return (val) ? 1 : 0;
    }

    int64_t ReadBits(size_t bits)
    {
        int64_t val = 0;
        if (0 != curr_save_point_.bit_pos)
        {
            val = *curr_save_point_.ptr & kBitMasks[curr_save_point_.bit_pos];
            if (curr_save_point_.bit_pos + bits < 8)
            {
                val >>= (8 - curr_save_point_.bit_pos - bits);
                curr_save_point_.bit_pos += bits;
                return val;
            }

            bits -= (8 - curr_save_point_.bit_pos);
            curr_save_point_.bit_pos = 0;
            ++curr_save_point_.ptr;
        }

        for (size_t bytes = bits >> 3; bytes > 0; --bytes)
        {
            val = (val << 8) + *curr_save_point_.ptr;
            ++curr_save_point_.ptr;
        }
        bits &= 0b111;

        if (bits > 0)
        {
            val = (val << bits) + (*curr_save_point_.ptr >> (8 - bits));
            curr_save_point_.bit_pos = bits;
        }

        return val;
    }

    int64_t ReadUe()
    {
        int64_t leading_zero_bits = 0;
        while (0 == ReadBit())
        {
            ++leading_zero_bits;
        }
        return (1LL << leading_zero_bits) + ReadBits(leading_zero_bits) - 1;
    }

    int64_t ReadSe()
    {
        int64_t val = ReadUe();
        if (0 == val % 2)
        {
            val = -val / 2;
        }
        else
        {
            val = val / 2 + 1;
        }
        return val;
    }

    void ReadAlign()
    {
        if (0 != curr_save_point_.bit_pos)
        {
            ++curr_save_point_.ptr;
            curr_save_point_.bit_pos = 0;
        }
    }

    // byte
    int64_t ReadByte()
    {
        return ReadBits(8);
    }

    int64_t Read2Bytes()
    {
        int64_t low_val = 0;
        int64_t high_val = 0;
        switch (byte_order_)
        {
        case ByteOrders::kBe:
            high_val = ReadByte();
            low_val = ReadByte();
            break;

        case ByteOrders::kLe:
            low_val = ReadByte();
            high_val = ReadByte();
            break;

        default:
            break;
        }

        return (high_val << 8) + low_val;
    }

    int64_t Read4Bytes()
    {
        int64_t low_val = 0;
        int64_t high_val = 0;
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

    int64_t Read8Bytes()
    {
        int64_t low_val = 0;
        int64_t high_val = 0;
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

    // byte stream
    void ReadByteStream(uint8_t *buf, size_t size)
    {
        if (0 == curr_save_point_.bit_pos)
        {
            memcpy(buf, curr_save_point_.ptr, size);
            curr_save_point_.ptr += size;
        }
        else
        {
            for (; size > 0; --size)
            {
                *buf++ = static_cast<uint8_t>(ReadByte());
            }
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

    const uint8_t *CurrPtr() const
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

LCCL_BIT_END_NAMESPACE
LCCL_END_NAMESPACE

#endif // !LCCL_INCLUDE_LCCL_BIT_READER_H_
