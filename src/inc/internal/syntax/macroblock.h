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

#define MB_TYPE_INTRA4x4 (1 << 0)   //1 << 0
#define MB_TYPE_INTRA16x16 (1 << 1) //1 << 1
#define MB_TYPE_INTRA_PCM (1 << 2)  //1 << 2

#define MB_TYPE_SI (1 << 3)

// {Part: consider 16x8 or 8x16 } {List}
#define MB_TYPE_P0L0 (1 << 4)
#define MB_TYPE_P1L0 (1 << 5)
#define MB_TYPE_P0L1 (1 << 6)
#define MB_TYPE_P1L1 (1 << 7)

#define MB_TYPE_16x16 (1 << 8)
#define MB_TYPE_16x8 (1 << 9)
#define MB_TYPE_8x16 (1 << 10)
#define MB_TYPE_8x8 (1 << 11)
#define MB_TYPE_8x8REF0 (1 << 12)
#define MB_TYPE_SKIP (1 << 13)

#define MB_TYPE_8x8DCT (1 << 20)

#define IS_INTRA4x4(a) ((a)&MB_TYPE_INTRA4x4)
#define IS_INTRA16x16(a) ((a)&MB_TYPE_INTRA16x16)
#define IS_INTRA_PCM(a) ((a)&MB_TYPE_INTRA_PCM)

#define IS_INTRA(a) ((a) & (MB_TYPE_INTRA4x4 | MB_TYPE_INTRA16x16 | MB_TYPE_INTRA_PCM))
#define IS_INTER(a) ((a) & (MB_TYPE_16x16 | MB_TYPE_16x8 | MB_TYPE_8x16 | MB_TYPE_8x8))
#define IS_SKIP(a) ((a)&MB_TYPE_SKIP)
#define IS_8x8DCT(a) ((a)&MB_TYPE_8x8DCT)

#define IS_16x16(a) ((a)&MB_TYPE_16x16)
#define IS_16x8(a) ((a)&MB_TYPE_16x8)
#define IS_8x16(a) ((a)&MB_TYPE_8x16)
#define IS_8x8(a) ((a)&MB_TYPE_8x8)
#define IS_8x8(a) ((a)&MB_TYPE_8x8)
#define IS_8x8REF0(a) ((a)&MB_TYPE_8x8REF0)
#define IS_SKIP(a) ((a)&MB_TYPE_SKIP)

enum IntraChromaPredMode
{
    DC_PRED_8 = 0,
    HOR_PRED_8 = 1,
    VERT_PRED_8 = 2,
    PLANE_8 = 3
};

struct IMbInfo
{
    int     type;
    int32_t pred_mode;
    int32_t cbp; // CodedBlockPatternChroma = cbp>>4; CodedBlockPatternLuma = cbp&0xf;
};
extern const IMbInfo ff_h264_i_mb_type_info[26];

// works for both P and B Macroblock
struct RefMbInfo
{
    int type;
    int partion_count;
};

struct macroblock_t
{
    int     _mb_type_;              // raw mb_type, note diffs with Deduced::MbType
    int32_t mb_field_decoding_flag; //equal to 0 specifies that the current macroblock pair is a frame macroblock pair

    int transform_size_8x8_flag;
    // prev_intra_[4x4|8x8]_pred_mode_flag
    bool prev_intra_nxn_pred_mode_flag[16];
    // rem_intra_[4x4|8x8]_pred_mode
    uint8_t rem_intra_nxn_pred_mode[16];

    IntraChromaPredMode intra_chroma_pred_mode;

    int coded_block_pattern;
    int mb_qp_delta;
};

struct Slice_t;
struct Macroblock_t : public macroblock_t
{
    Slice_t* slice;
    IMbInfo  i_mb_info;

    int MbType;
    struct Deduced
    {
        // $spec Table 7-15 – Specification of CodedBlockPatternChroma values
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