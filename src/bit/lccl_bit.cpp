#include "bit/bit_reader.h"
#include "bit/bit_writer.h"

LCCL_BEGIN_NAMESPACE
LCCL_BIT_BEGIN_NAMESPACE

std::shared_ptr<IBitReader> CreateBitReader(const uint8_t *data, size_t data_size, ByteOrders byte_order)
{
    if (!data)
    {
        return nullptr;
    }

    return std::make_shared<BitReader>(data, data_size, byte_order);
}

std::shared_ptr<IBitWriter> CreateBitWriter(size_t buf_size, ByteOrders byte_order)
{
    return std::make_shared<BitWriter>(nullptr, buf_size, byte_order);
}

std::shared_ptr<IBitWriter> CreateBitWriterWithBuffer(uint8_t *buf, size_t buf_size, ByteOrders byte_order)
{
    if (!buf)
    {
        return nullptr;
    }

    return std::make_shared<BitWriter>(buf, buf_size, byte_order);
}

LCCL_BIT_END_NAMESPACE
LCCL_END_NAMESPACE
