#pragma once
#include <stdint.h>
#include <math.h>
#include <log.h>
using byte = uint8_t;

typedef struct GetBitContext
{
    const uint8_t *buffer = nullptr, *buffer_end = nullptr;
    int            index = 0;
    int            size_in_bits = 0;
} GetBitContext;

static void InitGetBitContext(GetBitContext* gbc, const uint8_t* buffer, const uint8_t* buffer_end, int init_index = 0)
{
    gbc->buffer = buffer;
    gbc->buffer_end = buffer_end;
    gbc->index = init_index;
    gbc->size_in_bits = (gbc->buffer && gbc->buffer) ? (gbc->buffer_end - gbc->buffer) * 8 : 0;
}

inline static uint32_t Swap32(uint32_t u32)
{
    return (u32 >> 24) | (u32 >> 8 & 0xff00) | (u32 << 24) | (u32 << 8 & 0xff0000);
}

// prerequest: (gbc->index & 7) + n <= 32. so n <= 25 is safe
static uint32_t get_bits(GetBitContext* gbc, int n)
{
    const uint32_t buffer32 = Swap32(*(const uint32_t*)(gbc->buffer + (gbc->index >> 3)));
    int            bitOffset = gbc->index & 0x7;
    uint32_t       ret = (buffer32) << bitOffset >> (32 - n);
    gbc->index += n;
    return ret;
}

static uint32_t show_bits(GetBitContext* gbc, int n)
{
    const uint32_t buffer32 = Swap32(*(const uint32_t*)(gbc->buffer + (gbc->index >> 3)));
    int            bitOffset = gbc->index & 0x7;
    uint32_t       ret = (buffer32) << bitOffset >> (32 - n);
    return ret;
}

static void skip_bits(GetBitContext* gbc, int n)
{
    if (gbc->index + n <= gbc->size_in_bits)
        gbc->index += n;
}

/****************************************************************************************/
// plain params
/****************************************************************************************/
static uint32_t get_bits(const byte* buffer, int& bitCounter, int n)
{
    AASSERT(((bitCounter & 7) + n) <= 32);
    const uint32_t buffer32 = Swap32(*(const uint32_t*)(buffer + (bitCounter >> 3)));
    int            bitOffset = bitCounter & 0x7;
    uint32_t       ret = (buffer32) << bitOffset >> (32 - n);
    bitCounter += n;
    return ret;
}

static uint32_t show_bits(const byte* buffer, int& bitCounter, int n)
{
    AASSERT(((bitCounter & 7) + n) <= 32);
    const uint32_t buffer32 = Swap32(*(const uint32_t*)(buffer + (bitCounter >> 3)));
    int            bitOffset = bitCounter & 0x7;
    uint32_t       ret = (buffer32) << bitOffset >> (32 - n);
    return ret;
}

static void skip_bits(const byte* buffer, int& bitCounter, int n)
{
    bitCounter += n;
}

// for the range = [1, 2^13 - 2]
static uint32_t read_ue_golomb(const byte* buffer, int& bitCounter)
{
    uint32_t byteOffset = bitCounter >> 3;
    uint32_t byteOffset1 = bitCounter & 0x7;
    uint32_t cache = Swap32(*(uint32_t*)(buffer + byteOffset));
    // cache = cache << byteOffset1 >> byteOffset1;
    cache = cache << byteOffset1; // 0n 1 Xn ...
    int lg2 = log2(cache);
    int n = 32 - lg2 - 1;
    bitCounter += 2 * n + 1;
    if (n == 0)
        return 0;
    else
    {
        uint32_t ret = cache << (n + 1) >> (32 - n);
        ret += pow(2, n) - 1;
        return ret;
    }
}

static uint32_t read_se_golomb(const byte* buffer, int& bitCounter)
{
    auto ue = read_ue_golomb(buffer, bitCounter);
    if (ue & 1)
        return ue >> 1;
    else
        return -int32_t(ue >> 1);
}

/// @note $spec[7.4.1] rbsp_trailing_bits
static bool more_rbsp_data(const uint8_t* buffer, int bitOffset, int byteCnt)
{
    int byteOffset = bitOffset / 8;
    // ending happens at last byte
    if (byteOffset < byteCnt - 1)
        return true;

    // there must be a position for control bit
    AASSERT(bitOffset != byteCnt);

    uint8_t curByte = buffer[byteOffset];
    int     contorlBit_offset = bitOffset & 7;

    // control bit should be 1, remaining bits should be 0
    int controlBit = (curByte >> (7 - contorlBit_offset)) & 0x01;
    if (controlBit != 1)
        return true;
    int remainingBits = curByte & (1 << (7 - contorlBit_offset) - 1);

    return remainingBits;
}

#define StartBits(buf, bitCounter, bitlength)                                                                          \
    const uint8_t* _buf = buf;                                                                                         \
    int&           _bitCounter = bitCounter;                                                                           \
    int            _bitlength = bitlength
#define StartBitsCursor(cursor) StartBits((cursor).buf, (cursor).bit_pos, (cursor).bit_length)
#define GetBits(n) get_bits(_buf, _bitCounter, n)
#define SkipBits(n) skip_bits(_buf, _bitCounter, n)
#define MORE_RBSP_DATA() more_rbsp_data(_buf, _bitCounter, _bitlength / 8)
#define read_ue() read_ue_golomb(_buf, _bitCounter)
#define read_se() read_se_golomb(_buf, _bitCounter)