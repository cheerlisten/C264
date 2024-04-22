/* 
 * MBType*: ffmpeg/libavcodec/mpegutils.h 
*/
#pragma once
#include <stdint.h>
#include <vector>
struct BlockPos
{
    short x;
    short y;
};

#define MT_INTRA4x4 1   //1 << 0
#define MT_INTRA16x16 2 //1 << 1
#define MT_INTRA_PCM 4  //1 << 2

#define IS_MT_INTRA(mbType) ((mbType) & (MT_INTRA4x4 | MT_INTRA16x16 | MT_INTRA_PCM))
#define IS_MT_INTRA4x4(mbType) ((mbType)&MT_INTRA4x4)
#define IS_MT_INTRA16x16(mbType) ((mbType)&MT_INTRA16x16)
#define IS_MT_INTRA_PCM(mbType) ((mbType)&MT_INTRA_PCM)

struct IMbInfo
{
    int     type;
    int32_t pred_mode;
    int32_t cbp; // CodedBlockPatternChroma = cbp>>4; CodedBlockPatternLuma = cbp&0xf;
};
extern const IMbInfo ff_h264_i_mb_type_info[26];

struct macroblock_t
{
    int     mb_type;
    int32_t mb_field_decoding_flag; //equal to 0 specifies that the current macroblock pair is a frame macroblock pair

    int coded_block_pattern;
    int transform_size_8x8_flag;
    int mb_qp_delta;
};

struct Slice_t;
struct Macroblock_t : public macroblock_t
{
    Slice_t* slice;
    IMbInfo  i_mb_info;

    struct Deduced
    {
        int8_t CodedBlockPatternLuma;
        int8_t CodedBlockPatternChroma;
    } dd;

    uint8_t* pixels;
    /* debug views, better be natvis */
    uint8_t (*pixs8bit)[16][16] = *(decltype(pixs8bit)*)&pixels;
    uint8_t (*pixs1xbit)[16][16] = *(decltype(pixs1xbit)*)&pixels;
#define pixs8bit X_X_X
#define pixs1xbit Y_Y_Y
};

int32_t DecodeBlock_I_CABAC(Macroblock_t* curMB);
int32_t DecodeBlock_I_CAVLC(Macroblock_t* curMB);

void decode_I_PCM_samples(const uint8_t* dst_pixs,
                          int            bitDepth,
                          const uint8_t* databuf,
                          const uint8_t* databuf_end,
                          int            mbWidthC,
                          int            mbHeightC);