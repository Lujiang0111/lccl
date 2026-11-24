#ifndef LCCL_INCLUDE_LCCL_BIT_WRITER_H_
#define LCCL_INCLUDE_LCCL_BIT_WRITER_H_

#include <functional>
#include "lccl/bit/defines.h"

LCCL_NAMESPACE_BEGIN
LCCL_BIT_NAMESPACE_BEGIN

class BitWriter
{
public:
    using ResizeFunc = std::function<uint8_t *(void *opaque, size_t size)>;

    struct Pointer
    {
        size_t offset = 0;
        size_t bits = 0;
    };

public:
    BitWriter(uint8_t *data, size_t size,
        ResizeFunc resize_func, void *resize_func_opaque,
        ByteOrders byte_order = ByteOrders::kBe) :
        data_{ data },
        size_{ size },
        resize_func_{ resize_func },
        resize_func_opaque_{ resize_func_opaque },
        byte_order_{ byte_order }
    {

    }

    virtual ~BitWriter() = default;

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
        return (0 == pointer_.bits) ? (pointer_.offset) : (pointer_.offset + 1);
    }

    void Skip(size_t bytes, size_t bits)
    {
        pointer_.offset += bytes;
        pointer_.bits += bits;
        pointer_.offset += (pointer_.bits >> 3);
        pointer_.bits &= 0b111;

        size_t new_size = Size();
        if (new_size <= size_)
        {
            return;
        }

        data_ = resize_func_(resize_func_opaque_, new_size);
        size_ = new_size;
    }

    void WriteBits(uint64_t val, size_t bits)
    {
        if (0 == bits)
        {
            return;
        }

        Pointer start_pointer = pointer_;
        Skip(0, bits);

        if (start_pointer.bits + bits < 8)
        {
            data_[start_pointer.offset] &= (kNegBitMasks[start_pointer.bits] | kBitMasks[start_pointer.bits + bits]);
            data_[start_pointer.offset] |= ((val & kBitMasks[8 - bits]) << (8 - start_pointer.bits - bits));
            return;
        }

        if (0 != pointer_.bits)
        {
            data_[pointer_.offset] &= kBitMasks[pointer_.bits];
            data_[pointer_.offset] |= ((val & kBitMasks[8 - pointer_.bits]) << (8 - pointer_.bits));
            val >>= pointer_.bits;
        }

        for (size_t offset = pointer_.offset - 1; offset != start_pointer.offset; --offset)
        {
            data_[offset] = (val & 0xff);
            val >>= 8;
        }

        data_[start_pointer.offset] &= kNegBitMasks[start_pointer.bits];
        data_[start_pointer.offset] |= (val & kBitMasks[start_pointer.bits]);
    }

    void WriteUe(uint64_t val)
    {
        size_t bits = 0;
        for (uint64_t tmp = val + 1; 0 != tmp; tmp >>= 1)
        {
            ++bits;
        }
        bits += (bits - 1);

        WriteBits(val + 1, bits);
    }

    void WriteSe(int64_t val)
    {
        WriteUe((val > 0) ?
            ((static_cast<uint64_t>(val) << 1) - 1) :
            (static_cast<uint64_t>(-val) << 1));
    }

    void WriteAlign(uint64_t padding_bit)
    {
        if (0 != pointer_.bits)
        {
            if (0 == padding_bit)
            {
                data_[pointer_.offset] &= kNegBitMasks[pointer_.bits];
            }
            else
            {
                data_[pointer_.offset] |= kBitMasks[pointer_.bits];
            }
            
            ++pointer_.offset;
            pointer_.bits = 0;
        }
    }

    void Write1Byte(uint64_t val)
    {
        if (0 == pointer_.bits)
        {
            Skip(1, 0);
            data_[pointer_.offset - 1] = val & 0xff;
            return;
        }

        WriteBits(val, 8);
    }

    void Write2Bytes(uint64_t val)
    {
        uint64_t low_val = val;
        uint64_t high_val = val >> 8;
        switch (byte_order_)
        {
        case ByteOrders::kBe:
            Write1Byte(high_val);
            Write1Byte(low_val);
            break;

        case ByteOrders::kLe:
            Write1Byte(low_val);
            Write1Byte(high_val);
            break;

        default:
            break;
        }
    }

    void Write4Bytes(uint64_t val)
    {
        uint64_t low_val = val;
        uint64_t high_val = val >> 16;
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

    void Write8Bytes(uint64_t val)
    {
        uint64_t low_val = val;
        uint64_t high_val = val >> 32;
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

    void WriteByteStream(const uint8_t *data, size_t size)
    {
        if (0 == pointer_.bits)
        {
            Skip(size, 0);
            memcpy(data_ + pointer_.offset - size, data, size);
            return;
        }

        for (; size > 0; --size)
        {
            WriteBits(data[0], 8);
            ++data;
        }
    }

private:
    uint8_t *data_;
    size_t size_;
    ResizeFunc resize_func_;
    void *resize_func_opaque_;
    ByteOrders byte_order_;

    Pointer pointer_;
};

LCCL_BIT_NAMESPACE_END
LCCL_NAMESPACE_END

#endif // !LCCL_INCLUDE_LCCL_BIT_WRITER_H_
