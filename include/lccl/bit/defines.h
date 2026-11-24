#ifndef LCCL_INCLUDE_LCCL_BIT_DEFINES_H_
#define LCCL_INCLUDE_LCCL_BIT_DEFINES_H_

#include <cstdint>
#include <cstring>
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
    0b11111111,     // 0
    0b01111111,     // 1
    0b00111111,     // 2
    0b00011111,     // 3
    0b00001111,     // 4
    0b00000111,     // 5
    0b00000011,     // 6
    0b00000001,     // 7
    0b00000000,     // 8
};

constexpr uint8_t kNegBitMasks[] =
{
    0b00000000,     // 0
    0b10000000,     // 1
    0b11000000,     // 2
    0b11100000,     // 3
    0b11110000,     // 4
    0b11111000,     // 5
    0b11111100,     // 6
    0b11111110,     // 7
    0b11111111,     // 8
};

LCCL_BIT_NAMESPACE_END
LCCL_NAMESPACE_END

#endif // !LCCL_INCLUDE_LCCL_BIT_DEFINES_H_
