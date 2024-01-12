
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <vector>

#define Log(...) printf(__VA_ARGS__)
using byte = uint8_t;

//! values for nal_unit_type
typedef enum
{
    NALU_TYPE_SLICE = 1,
    NALU_TYPE_DPA = 2,
    NALU_TYPE_DPB = 3,
    NALU_TYPE_DPC = 4,
    NALU_TYPE_IDR = 5,
    NALU_TYPE_SEI = 6,
    NALU_TYPE_SPS = 7,
    NALU_TYPE_PPS = 8,
    NALU_TYPE_AUD = 9,
    NALU_TYPE_EOSEQ = 10,
    NALU_TYPE_EOSTREAM = 11,
    NALU_TYPE_FILL = 12,
#if (MVC_EXTENSION_ENABLE)
    NALU_TYPE_PREFIX = 14,
    NALU_TYPE_SUB_SPS = 15,
    NALU_TYPE_SLC_EXT = 20,
    NALU_TYPE_VDRD = 24 // View and Dependency Representation Delimiter NAL Unit
#endif
} NaluType;

struct GetBitContext
{
    // primary data
    const uint8_t *buffer, *buffer_end;
    // state
    int index;
    // side data
    int size_in_bits;
};

uint32_t Swap32(uint32_t u32)
{
    return (u32 >> 24) | (u32 >> 8 & 0xff00) | (u32 << 24) | (u32 << 8 & 0xff0000);
}

// prerequest: (gbc->index & 7) + n <= 32. so n <= 25 is safe
static inline uint32_t get_bits(GetBitContext* gbc, int n)
{
    const uint32_t buffer32 = Swap32(*(const uint32_t*)(gbc->buffer + (gbc->index >> 3)));
    int bitOffset = gbc->index & 0x7;
    uint32_t ret = (buffer32) << bitOffset >> (32 - n);
    gbc->index += n;
    return ret;
}
static inline uint32_t show_bits(GetBitContext* gbc, int n)
{
    const uint32_t buffer32 = Swap32(*(const uint32_t*)(gbc->buffer + (gbc->index >> 3)));
    int bitOffset = gbc->index & 0x7;
    uint32_t ret = (buffer32) << bitOffset >> (32 - n);
    return ret;
}
static inline void skip_bits(GetBitContext* gbc, int n)
{
    if (gbc->index + n <= gbc->size_in_bits)
        gbc->index += n;
}

int32_t nextStartCode(byte* buf, int start_index, int buf_sz, int* start_code_length = nullptr)
{
    GetBitContext _gbc;
    _gbc.buffer = buf;
    _gbc.buffer_end = buf + buf_sz;
    _gbc.size_in_bits = buf_sz * 8;
    _gbc.index = start_index * 8;
    GetBitContext* gbc = &_gbc;
    // leading zero
    int default_lzc;
    while (_gbc.index < _gbc.size_in_bits)
    {
        int i32 = show_bits(gbc, 24);
        if (i32 == 0x000001)
        {
            if (start_code_length)
                if (gbc->index)
                    *start_code_length = 3 + gbc->buffer[gbc->index / 8 - 1] == 0x00;
                else
                    *start_code_length = 3;
            break;
        }
        skip_bits(gbc, 8);
    }
    return show_bits(gbc, 24) == 0x000001 ? gbc->index / 8 : -1u;
}

struct NAL_SEI_MSG
{
};

struct NAL_SEI
{
    std::vector<NAL_SEI_MSG> msgs;
};

int main()
{
    FILE* h264File = fopen(CMAKE_SOURCE_DIR "/../C264-Resource/Res/USA20.h264", "rb");
#define CACHE_SZ 4096 * 10
    byte cache[CACHE_SZ];
    size_t freadSize = fread(cache, 1, CACHE_SZ, h264File);
    GetBitContext* gbc = new GetBitContext;
    gbc->buffer = cache;
    gbc->buffer_end = cache + CACHE_SZ;
    gbc->index = 0;
    gbc->size_in_bits = CACHE_SZ * 8;

    int leading_zero_count;
    int nextStartCodePos = 0; // nextStartCode(cache, 0, CACHE_SZ);
    skip_bits(gbc, nextStartCodePos * 8);

    while (1)
    {
        nextStartCodePos = nextStartCode(cache, gbc->index / 8 + 3, CACHE_SZ);
        if (nextStartCodePos == -1u)
        {
            Log("reach end");
            break;
        }
        leading_zero_count = 0;
        while (show_bits(gbc, 24) != 0x000001)
        {
            ++leading_zero_count;
            skip_bits(gbc, 8);
        }
        int numBytesInNALUnit = nextStartCodePos - gbc->index / 8;

        // start_code
        int startCode = get_bits(gbc, 24);

        // start nalu
        int forbidden_zero_bit = get_bits(gbc, 1);
        int nal_ref_idc = get_bits(gbc, 2);
        NaluType nal_unit_type = (NaluType)get_bits(gbc, 5);
        Log("found NalType = %d at %d\n", nal_unit_type, gbc->index / 8 - 4);

        switch (nal_unit_type)
        {
        case NALU_TYPE_SEI:
        {
            NAL_SEI sei;
            do
            {
                NAL_SEI_MSG msg;

                uint32_t payloadType = 0;
                while (show_bits(gbc, 8) == 0xff)
                {
                    payloadType += 255;
                    skip_bits(gbc, 8);
                }
                byte last_payload_type_byte = get_bits(gbc, 8);
                payloadType += last_payload_type_byte;

                int payloadSize = 0;
                while (show_bits(gbc, 8) == 0xff)
                {
                    payloadSize += 255;
                    skip_bits(gbc, 8);
                }
                byte last_payload_size_byte = get_bits(gbc, 8);
                payloadSize += last_payload_size_byte;

                // sei_payload(payloadType, payloadSize)
                skip_bits(gbc, payloadSize * 8);
            } while (show_bits(gbc, 8) != 0x80);
            skip_bits(gbc, 8); // the last RBSP sanitel
            break;
        }
        default:
            skip_bits(gbc, (numBytesInNALUnit - 4) * 8);
            break;
        }
    }
    return 0;
}
