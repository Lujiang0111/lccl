#ifndef LCCL_INTERNAL_BIT_BIT_READER_H_
#define LCCL_INTERNAL_BIT_BIT_READER_H_

#include <unordered_map>
#include "lccl/bit.h"

LCCL_BEGIN_NAMESPACE
LCCL_BIT_BEGIN_NAMESPACE

class BitReader : public IBitReader
{
public:
    BitReader() = delete;
    BitReader(const BitReader &) = delete;
    BitReader &operator=(const BitReader &) = delete;

    BitReader(const uint8_t *data, size_t data_size, ByteOrders byte_order);
    virtual ~BitReader();

    // bit
    virtual int64_t ReadBits(size_t bits);
    virtual int64_t ReadUe();
    virtual int64_t ReadSe();
    virtual void ReadAlign();

    // byte
    virtual int64_t ReadByte();
    virtual int64_t Read2Bytes();
    virtual int64_t Read4Bytes();
    virtual int64_t Read8Bytes();

    // byte stream
    virtual void ReadBytes(uint8_t *buf, size_t size);

    // skip
    virtual void SkipBits(size_t bits);
    virtual void SkipBytes(size_t bytes);

    // save/load
    virtual int64_t Save();
    virtual bool Load(int64_t saved_point);
    virtual void ClearSave(int64_t saved_point);

    virtual size_t Size() const;
    virtual bool IsAvailable(size_t size) const;

private:
    struct SavePoint
    {
        size_t offset = 0;
        uint8_t bit_mask = 0x80;
    };

private:
    void NextBitMask();

private:
    const uint8_t *data_;
    size_t data_size_;
    ByteOrders byte_order_;

    SavePoint cur_save_point_;

    int64_t next_save_point_id_;
    std::unordered_map<int64_t, SavePoint> save_points_;
};

LCCL_BIT_END_NAMESPACE
LCCL_END_NAMESPACE

#endif // !LCCL_INTERNAL_BIT_BIT_READER_H_
