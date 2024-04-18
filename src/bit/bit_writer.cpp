#include "bit/bit_writer.h"

LCCL_BEGIN_NAMESPACE
LCCL_BIT_BEGIN_NAMESPACE

constexpr size_t kBufferPadingSize = 1024;

BitWriter::BitWriter(uint8_t *buf, size_t buf_size, ByteOrders byte_order) :
    buf_(buf),
    buf_size_(buf_size),
    byte_order_(byte_order),
    internal_buf_(0),
    next_save_point_id_(0)
{
    if (!buf_)
    {
        internal_buf_.assign(kBufferPadingSize, 0);
        buf_ = &internal_buf_[0];
    }
}

BitWriter::~BitWriter()
{

}

void BitWriter::WriteBits(int64_t val, size_t bits)
{
    ResizeInternalBuf(bits / 8 + 1);
    for (--bits; bits > 0; --bits)
    {
        if ((val >> bits) & 0x01)
        {
            buf_[cur_save_point_.offset] |= cur_save_point_.bit_mask;
        }
        else
        {
            buf_[cur_save_point_.offset] &= ~cur_save_point_.bit_mask;
        }
        NextBitMask();
    }
}

void BitWriter::WriteUe(int64_t val)
{
    size_t bits = 0;
    for (int64_t tmp = val + 1; tmp > 0; tmp >>= 1)
    {
        ++bits;
    }
    bits += (bits - 1);

    WriteBits(val + 1, bits);
}

void BitWriter::WriteSe(int64_t val)
{
    WriteUe((val > 0) ? (2 * val - 1) : (-val * 2));
}

void BitWriter::WriteAlign()
{
    while (0x80 != cur_save_point_.bit_mask)
    {
        WriteBits(0, 1);
    }
}

void BitWriter::WriteByte(int64_t val)
{
    ResizeInternalBuf(1);
    if (0x80 == cur_save_point_.bit_mask)
    {
        buf_[cur_save_point_.offset] = val & 0xFF;
        ++cur_save_point_.offset;
    }
    else
    {
        WriteBits(val, 8);
    }
}

void BitWriter::Write2Bytes(int64_t val)
{
    int64_t low_val = val;
    int64_t high_val = val >> 8;
    switch (byte_order_)
    {
    case ByteOrders::kLe:
        WriteByte(low_val);
        WriteByte(high_val);
        break;

    case ByteOrders::kBe:
        WriteByte(high_val);
        WriteByte(low_val);
        break;

    default:
        break;
    }
}

void BitWriter::Write4Bytes(int64_t val)
{
    int64_t low_val = val;
    int64_t high_val = val >> 16;
    switch (byte_order_)
    {
    case ByteOrders::kLe:
        Write2Bytes(low_val);
        Write2Bytes(high_val);
        break;

    case ByteOrders::kBe:
        Write2Bytes(high_val);
        Write2Bytes(low_val);
        break;

    default:
        break;
    }
}

void BitWriter::Write8Bytes(int64_t val)
{
    int64_t low_val = val;
    int64_t high_val = val >> 32;
    switch (byte_order_)
    {
    case ByteOrders::kLe:
        Write4Bytes(low_val);
        Write4Bytes(high_val);
        break;

    case ByteOrders::kBe:
        Write4Bytes(high_val);
        Write4Bytes(low_val);
        break;

    default:
        break;
    }
}

void BitWriter::WriteBytes(const uint8_t *data, size_t size)
{
    if (0x80 != cur_save_point_.bit_mask)
    {
        for (; size > 0; --size)
        {
            WriteByte(*data);
            ++data;
        }
        return;
    }

    ResizeInternalBuf(size);
    memcpy(buf_ + cur_save_point_.offset, data, size);
    cur_save_point_.offset += size;
}

void BitWriter::SkipBits(size_t bits)
{
    SkipBytes(bits / 8);

    bits %= 8;
    if (bits > 0)
    {
        ResizeInternalBuf(1);
        for (; bits > 0; --bits)
        {
            NextBitMask();
        }
    }
}

void BitWriter::SkipBytes(size_t bytes)
{
    ResizeInternalBuf(bytes);
    cur_save_point_.offset += bytes;
}

int64_t BitWriter::Save()
{
    save_points_[next_save_point_id_] = cur_save_point_;
    return next_save_point_id_++;
}

bool BitWriter::Load(int64_t save_point_id_)
{
    auto it = save_points_.find(save_point_id_);
    if (save_points_.end() == it)
    {
        return false;
    }

    cur_save_point_ = it->second;
    return true;
}

void BitWriter::ClearSave(int64_t save_point_id_)
{
    auto it = save_points_.find(save_point_id_);
    if (save_points_.end() == it)
    {
        return;
    }

    save_points_.erase(it);
}

uint8_t *BitWriter::Buf()
{
    return buf_;
}

size_t BitWriter::Size() const
{
    return (0x80 == cur_save_point_.bit_mask) ? (cur_save_point_.offset) : (cur_save_point_.offset + 1);
}

bool BitWriter::IsAvailable(size_t size) const
{
    if (0 == buf_size_)
    {
        return true;
    }

    return (Size() + size < buf_size_);
}

void BitWriter::NextBitMask()
{
    cur_save_point_.bit_mask >>= 1;
    if (0 == cur_save_point_.bit_mask)
    {
        ++cur_save_point_.offset;
        cur_save_point_.bit_mask = 0x80;
    }
}

void BitWriter::ResizeInternalBuf(size_t bytes)
{
    if (internal_buf_.empty())
    {
        return;
    }

    if (cur_save_point_.offset + bytes >= internal_buf_.size())
    {
        internal_buf_.resize((std::max)(2 * internal_buf_.size(), cur_save_point_.offset + bytes + kBufferPadingSize));
        buf_ = &internal_buf_[0];
    }
}


LCCL_BIT_END_NAMESPACE
LCCL_END_NAMESPACE
