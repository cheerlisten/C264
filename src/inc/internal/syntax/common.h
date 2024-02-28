#pragma once
#include <memory>
#include <vector>

#include <bitGetter.h>

struct RefBuffer
{
    std::vector<uint8_t> data;
};

// RBSP
struct RefNalu : RefBuffer
{
    uint64_t source_pos;        // debug info
    int32_t  nalu_len;          // bytes length
    int      start_code_length; // 0x01 is in count
};

struct RBSPCursor
{
    RBSPCursor(std::shared_ptr<RefNalu> refNal_, int bitRangeStart = 0, int bitRangeLen = -1) :
        refNal(refNal_), buf(nullptr), bit_length(0), bit_pos(0)
    {
        if (refNal)
        {
            buf = refNal->data.data() + bitRangeStart / 8;
            bit_length = bitRangeLen == -1 ? refNal->nalu_len * 8 : bitRangeLen;
            bit_pos = bitRangeStart % 8;
        }
    }
    int BaseShiftBytes(int shiftedBytesLength)
    {
        if (unsigned(bit_pos - shiftedBytesLength * 8) < bit_length)
        {
            bit_pos -= shiftedBytesLength * 8;
            bit_length -= shiftedBytesLength * 8;
            buf += shiftedBytesLength;
            return 0;
        }
        else
            return -1;
    }
    std::shared_ptr<RefNalu> refNal;
    uint8_t*                 buf;
    int32_t                  bit_length;
    int32_t                  bit_pos;
    GetBitContext            GetGBC();
};

/// @brief de-escaping nalu data into rbsp
/// @param bufView nalu data
std::shared_ptr<RefNalu> makeRefNal(BufferView bufView);