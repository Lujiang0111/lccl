#include "bit/bit_reader.h"

LCCL_BEGIN_NAMESPACE
LCCL_BIT_BEGIN_NAMESPACE

BitReader::BitReader(const uint8_t *data, size_t data_size, ByteOrders byte_order) :
    data_(data),
    data_size_(data_size),
    byte_order_(byte_order),
    next_save_point_id_(0)
{

}

BitReader::~BitReader()
{

}

int64_t BitReader::ReadBits(size_t bits)
{
    int64_t val = 0;
    for (; bits > 0; --bits)
    {
        val = (val << 1) | ((data_[cur_save_point_.offset] & cur_save_point_.bit_mask) ? 1 : 0);
        NextBitMask();
    }
    return val;
}

int64_t BitReader::ReadUe()
{
    int64_t leading_zero_bits = 0;
    while (0 == ReadBits(1))
    {
        ++leading_zero_bits;
    }
    return (1LL << leading_zero_bits) + ReadBits(leading_zero_bits) - 1;
}

int64_t BitReader::ReadSe()
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

void BitReader::ReadAlign()
{
    if (0x80 != cur_save_point_.bit_mask)
    {
        ++cur_save_point_.offset;
        cur_save_point_.bit_mask = 0x80;
    }
}

int64_t BitReader::ReadByte()
{
    int64_t val = 0;
    if (0x80 == cur_save_point_.bit_mask)
    {
        val = data_[cur_save_point_.offset];
        ++cur_save_point_.offset;
    }
    else
    {
        val = ReadBits(8);
    }

    return val;
}

int64_t BitReader::Read2Bytes()
{
    int64_t low_val = 0;
    int64_t high_val = 0;
    switch (byte_order_)
    {
    case ByteOrders::kLe:
        low_val = ReadByte();
        high_val = ReadByte();
        break;

    case ByteOrders::kBe:
        high_val = ReadByte();
        low_val = ReadByte();
        break;

    default:
        break;
    }

    return (high_val << 8) + low_val;
}

int64_t BitReader::Read4Bytes()
{
    int64_t low_val = 0;
    int64_t high_val = 0;
    switch (byte_order_)
    {
    case ByteOrders::kLe:
        low_val = Read2Bytes();
        high_val = Read2Bytes();
        break;

    case ByteOrders::kBe:
        high_val = Read2Bytes();
        low_val = Read2Bytes();
        break;

    default:
        break;
    }

    return (high_val << 16) + low_val;
}

int64_t BitReader::Read8Bytes()
{
    int64_t low_val = 0;
    int64_t high_val = 0;
    switch (byte_order_)
    {
    case ByteOrders::kLe:
        low_val = Read4Bytes();
        high_val = Read4Bytes();
        break;

    case ByteOrders::kBe:
        high_val = Read4Bytes();
        low_val = Read4Bytes();
        break;

    default:
        break;
    }

    return (high_val << 32) + low_val;
}

void BitReader::ReadBytes(uint8_t *buf, size_t size)
{
    if (0x80 != cur_save_point_.bit_mask)
    {
        for (; size > 0; --size)
        {
            *buf++ = static_cast<uint8_t>(ReadByte());
        }
        return;
    }

    memcpy(buf, data_ + cur_save_point_.offset, size);
    cur_save_point_.offset += size;
}

void BitReader::SkipBits(size_t bits)
{
    SkipBytes(bits / 8);

    bits %= 8;
    for (size_t i = 0; i < bits; ++i)
    {
        NextBitMask();
    }
}

void BitReader::SkipBytes(size_t bytes)
{
    cur_save_point_.offset += bytes;
}

int64_t BitReader::Save()
{
    save_points_[next_save_point_id_] = cur_save_point_;
    return next_save_point_id_++;
}

bool BitReader::Load(int64_t save_point_id_)
{
    auto it = save_points_.find(save_point_id_);
    if (save_points_.end() == it)
    {
        return false;
    }

    cur_save_point_ = it->second;
    return true;
}

void BitReader::ClearSave(int64_t save_point_id_)
{
    auto it = save_points_.find(save_point_id_);
    if (save_points_.end() == it)
    {
        return;
    }

    save_points_.erase(it);
}

size_t BitReader::Size() const
{
    return (0x80 == cur_save_point_.bit_mask) ? (cur_save_point_.offset) : (cur_save_point_.offset + 1);
}

bool BitReader::IsAvailable(size_t size) const
{
    if (0 == data_size_)
    {
        return true;
    }

    return (Size() + size < data_size_);
}


void BitReader::NextBitMask()
{
    cur_save_point_.bit_mask >>= 1;
    if (0 == cur_save_point_.bit_mask)
    {
        ++cur_save_point_.offset;
        cur_save_point_.bit_mask = 0x80;
    }
}

LCCL_BIT_END_NAMESPACE
LCCL_END_NAMESPACE
