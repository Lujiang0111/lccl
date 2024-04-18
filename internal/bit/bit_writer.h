#ifndef LCCL_INTERNAL_BIT_BIT_WRITER_H_
#define LCCL_INTERNAL_BIT_BIT_WRITER_H_

#include <unordered_map>
#include <vector>
#include "lccl/bit.h"

LCCL_BEGIN_NAMESPACE
LCCL_BIT_BEGIN_NAMESPACE

class BitWriter : public IBitWriter
{
public:
    BitWriter() = delete;
    BitWriter(const BitWriter &) = delete;
    BitWriter &operator=(const BitWriter &) = delete;

    BitWriter(uint8_t *buf, size_t buf_size, ByteOrders byte_order);
    virtual ~BitWriter();

    // bit
    virtual void WriteBits(int64_t val, size_t bits);
    virtual void WriteUe(int64_t val);
    virtual void WriteSe(int64_t val);
    virtual void WriteAlign();

    // byte
    virtual void WriteByte(int64_t val);
    virtual void Write2Bytes(int64_t val);
    virtual void Write4Bytes(int64_t val);
    virtual void Write8Bytes(int64_t val);

    // byte stream
    virtual void WriteBytes(const uint8_t *data, size_t size);

    // skip
    virtual void SkipBits(size_t bits);
    virtual void SkipBytes(size_t bytes);

    // save/load
    virtual int64_t Save();
    virtual bool Load(int64_t saved_point);
    virtual void ClearSave(int64_t saved_point);

    virtual uint8_t *Buf();
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
    void ResizeInternalBuf(size_t bytes);

private:
    uint8_t *buf_;
    size_t buf_size_;
    ByteOrders byte_order_;

    std::vector<uint8_t> internal_buf_;
    SavePoint cur_save_point_;

    int64_t next_save_point_id_;
    std::unordered_map<int64_t, SavePoint> save_points_;
};

LCCL_BIT_END_NAMESPACE
LCCL_END_NAMESPACE

#endif // !LCCL_INTERNAL_BIT_BIT_WRITER_H_
