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
    // debug info
    uint64_t source_pos;
    int32_t  nalu_len;
    int      start_code_length; // 0x01 is in count
};

struct RBSPCursor
{
    RBSPCursor(std::shared_ptr<RefNalu> refNal_, int startPos = 0, int sz = -1, int bitPos = -1) :
        refNal(refNal_), buf(nullptr), size(0), bit_pos(0)
    {
        if (refNal)
        {
            buf = refNal->data.data() + startPos;
            size = sz == -1 ? refNal->data.size() - startPos : sz;
            bit_pos = bit_pos == -1 ? startPos * 8 : bit_pos;
        }
    }
    std::shared_ptr<RefNalu> refNal;
    uint8_t*                 buf;
    int32_t                  size;
    int32_t                  bit_pos;
    GetBitContext            GetGBC();
};

/// @brief de-escaping nalu data into rbsp
/// @param bufView nalu data
std::shared_ptr<RefNalu> makeRefNal(BufferView bufView);