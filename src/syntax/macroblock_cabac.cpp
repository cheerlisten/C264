#include <syntax/syntax.h>
#include <defines.h>
/* $spec[Table 7-11] Macroblock types for I slices */
const IMbInfo ff_h264_i_MB_TYPE_info[26] = {
    { MB_TYPE_INTRA4x4, -1, -1 },       { MB_TYPE_INTRA16x16, 0, 0 },       { MB_TYPE_INTRA16x16, 1, 0 },
    { MB_TYPE_INTRA16x16, 2, 0 },       { MB_TYPE_INTRA16x16, 3, 0 },       { MB_TYPE_INTRA16x16, 0, 16 },
    { MB_TYPE_INTRA16x16, 1, 16 },      { MB_TYPE_INTRA16x16, 2, 16 },      { MB_TYPE_INTRA16x16, 3, 16 },
    { MB_TYPE_INTRA16x16, 0, 32 },      { MB_TYPE_INTRA16x16, 1, 32 },      { MB_TYPE_INTRA16x16, 2, 32 },
    { MB_TYPE_INTRA16x16, 3, 32 },      { MB_TYPE_INTRA16x16, 0, 15 + 0 },  { MB_TYPE_INTRA16x16, 1, 15 + 0 },
    { MB_TYPE_INTRA16x16, 2, 15 + 0 },  { MB_TYPE_INTRA16x16, 3, 15 + 0 },  { MB_TYPE_INTRA16x16, 0, 15 + 16 },
    { MB_TYPE_INTRA16x16, 1, 15 + 16 }, { MB_TYPE_INTRA16x16, 2, 15 + 16 }, { MB_TYPE_INTRA16x16, 3, 15 + 16 },
    { MB_TYPE_INTRA16x16, 0, 15 + 32 }, { MB_TYPE_INTRA16x16, 1, 15 + 32 }, { MB_TYPE_INTRA16x16, 2, 15 + 32 },
    { MB_TYPE_INTRA16x16, 3, 15 + 32 }, { MB_TYPE_INTRA_PCM, -1, -1 },
};

/* try template for static dispatch */
FORCE_INLINE int decode_mb_type_cabac(Macroblock_t* curMB, Slice_t* slice)
{
    // $spec Table[9-34] - Syntax elements and associated types of binarization, maxBinIdxCtx, and ctxIdxOffset
    // $spec Table[9-39] - the assignment of ctxIdx increments (ctxIdxInc) to binIdx for all ctxIdxOffset
    // $spec Table[9-36] - Binarization for macroblock types in I slices
    // $spec Chp[9.3.3.1.1.3] - Derivation process of ctxIdxInc for the syntax element mb_type

    const int ctxIdxOffset = 3;
    int       ctxInc0 = ctxIdxOffset;
    // Since neighbor's existence is not guaranted, so as the mbType.
    // So this is the right version of 9.3.3.1.1.3
    if (slice->mbTypeA & (MT_INTRA16x16 | MT_INTRA_PCM))
        ctxInc0++;
    if (slice->mbTypeB & (MT_INTRA16x16 | MT_INTRA_PCM))
        ctxInc0++;

    if (0 == slice->cabac.decoder.DecodeDecision(slice->cabac.bac_contexts + ctxInc0))
        return 0; // I_NxN

    if (slice->cabac.decoder.DecodeTermination())
        return 25; // I_PCM

    int ret = 1;
    if (slice->cabac.decoder.DecodeDecision(slice->cabac.bac_contexts + ctxIdxOffset + 3))
        ret += 12;
    if (slice->cabac.decoder.DecodeDecision(slice->cabac.bac_contexts + ctxIdxOffset + 4))
    {
        ret += 4;
        if (slice->cabac.decoder.DecodeDecision(slice->cabac.bac_contexts + ctxIdxOffset + 5))
            ret += 4;
    }
    if (slice->cabac.decoder.DecodeDecision(slice->cabac.bac_contexts + ctxIdxOffset + 6))
        ret += 2;
    if (slice->cabac.decoder.DecodeDecision(slice->cabac.bac_contexts + ctxIdxOffset + 7))
        ret += 1;

    return ret;
}

// I Slice only
FORCE_INLINE int mb_pred(Macroblock_t* curMB, Slice_t* slice)
{
    AASSERT(!slice->dd.MbaffFrameFlag, "no mbaff");

    if (IS_MT_INTRA4x4(curMB->mb_type))
    {
        if (curMB->transform_size_8x8_flag)
            ;
    }
    return 0;
}

FORCE_INLINE int residual_cabac(Macroblock_t* curMB, Slice_t* slice)
{
    return 0;
}

// $spec Chp[7.3.5.1] Macroblock prediction syntax
int32_t DecodeBlock_I_CABAC(Macroblock_t* curMB)
{
    Slice_t* slice = curMB->slice;

    int raw_mb_type = decode_mb_type_cabac(curMB, slice);
    curMB->i_mb_info = ff_h264_i_MT_info[raw_mb_type];
    int noSubMbPartSizeLessThan8x8Flag = 1;

    //$spec E[7-36]
    curMB->dd.CodedBlockPatternLuma = curMB->i_mb_info.cbp / 16;
    curMB->dd.CodedBlockPatternChroma = curMB->i_mb_info.cbp % 16;
    int mbType = curMB->i_mb_info.type;

    if (IS_MT_INTRA_PCM(mbType))
    {
        /* Get align */
        const uint8_t* pcm_data_ptr = slice->cabac.decoder.buffer_cur - slice->cabac.decoder.leftBits / 8;
        int            pcm_data_size = slice->sps->dd.BitDepth_Y * (16 * 16 + 2 * slice->sps->dd.MbHeightC);
        const uint8_t* pcm_data_end_ptr = pcm_data_ptr + pcm_data_size;
        decode_I_PCM_samples(curMB->pixels,
                             slice->sps->dd.BitDepth_Y,
                             pcm_data_ptr,
                             pcm_data_end_ptr,
                             slice->sps->dd.MbWidthC,
                             slice->sps->dd.MbHeightC);
        slice->cabac.decoder.Init(pcm_data_end_ptr, slice->cabac.decoder.buffer_end - pcm_data_end_ptr);
        return 0;
    }

    // Prediction
    if (IS_MT_INTRA(mbType))
    {
        if (slice->pps->transform_8x8_mode_flag && IS_MT_INTRA4x4(mbType))
            AASSERT(!"@todo transform_size_8x8_flag");
        mb_pred(curMB, slice);
    }

    if (!(mbType & MT_INTRA16x16))
    {
        AASSERT(!"@todo coded_block_pattern");
        //$spec E[7-36]
        curMB->dd.CodedBlockPatternLuma = curMB->coded_block_pattern / 16;
        curMB->dd.CodedBlockPatternChroma = curMB->coded_block_pattern % 16;
        if (curMB->dd.CodedBlockPatternLuma > 0 && slice->pps->transform_8x8_mode_flag && !(mbType & MT_INTRA4x4) &&
            noSubMbPartSizeLessThan8x8Flag)
            AASSERT(!"@todo transform_size_8x8_flag");
    }

    // Dequant
    if (curMB->dd.CodedBlockPatternLuma > 0 || curMB->dd.CodedBlockPatternChroma > 0 || (mbType & MT_INTRA16x16))
    {
        AASSERT(!"@todo mb_qp_delta");
        residual_cabac(curMB, slice);
    }

    return 0;
}
