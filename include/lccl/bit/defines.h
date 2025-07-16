#ifndef LCCL_INCLUDE_LCCL_BIT_DEFINES_H_
#define LCCL_INCLUDE_LCCL_BIT_DEFINES_H_

#include "lccl.h"

#define LCCL_BIT_NAMESPACE_BEGIN namespace bit {
#define LCCL_BIT_NAMESPACE_END }

LCCL_NAMESPACE_BEGIN
LCCL_BIT_NAMESPACE_BEGIN

enum class ByteOrders
{
    kBe = 0,
    kLe,
};

constexpr uint8_t kBitMasks[] =
{
    0b11111111,
    0b01111111,
    0b00111111,
    0b00011111,
    0b00001111,
    0b00000111,
    0b00000011,
    0b00000001,
};

constexpr uint8_t kBitNegMasks[] =
{
    0b00000000,
    0b10000000,
    0b11000000,
    0b11100000,
    0b11110000,
    0b11111000,
    0b11111100,
    0b11111110,
    0b11111111,
};

constexpr uint8_t kBitPoints[] =
{
    0b10000000,
    0b01000000,
    0b00100000,
    0b00010000,
    0b00001000,
    0b00000100,
    0b00000010,
    0b00000001,
};

struct SavePoint
{
    uint8_t *ptr;
    size_t bit_pos;

    SavePoint(uint8_t *i_ptr = nullptr, size_t i_bit_pos = 0) :
        ptr(i_ptr),
        bit_pos(i_bit_pos)
    {

    }
};

LCCL_BIT_NAMESPACE_END
LCCL_NAMESPACE_END

#endif // !LCCL_INCLUDE_LCCL_BIT_DEFINES_H_
