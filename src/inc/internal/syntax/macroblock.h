/* 
 * MBType*: ffmpeg/libavcodec/mpegutils.h 
*/
#pragma once
#include <stdint.h>
struct BlockPos
{
    short x;
    short y;
};

#define MT_INTRA4x4 1 << 0
#define MT_INTRA16x16 1 << 1
#define MT_INTRA_PCM 1 << 2

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
};
struct Slice_t;
struct Macroblock_t : public macroblock_t
{
    Slice_t* slice;
    IMbInfo  i_mb_info;
};

int32_t DecodeBlock_I_CABAC(Macroblock_t* curMB);
int32_t DecodeBlock_I_CAVLC(Macroblock_t* curMB);