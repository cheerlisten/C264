#include <syntax/syntax.h>
#include <defines.h>
/* $spec[Table 7-11] Macroblock types for I slices */
const IMbInfo ff_h264_i_MT_info[26] = {
    { MT_INTRA4x4, -1, -1 },       { MT_INTRA16x16, 0, 0 },       { MT_INTRA16x16, 1, 0 },
    { MT_INTRA16x16, 2, 0 },       { MT_INTRA16x16, 3, 0 },       { MT_INTRA16x16, 0, 16 },
    { MT_INTRA16x16, 1, 16 },      { MT_INTRA16x16, 2, 16 },      { MT_INTRA16x16, 3, 16 },
    { MT_INTRA16x16, 0, 32 },      { MT_INTRA16x16, 1, 32 },      { MT_INTRA16x16, 2, 32 },
    { MT_INTRA16x16, 3, 32 },      { MT_INTRA16x16, 0, 15 + 0 },  { MT_INTRA16x16, 1, 15 + 0 },
    { MT_INTRA16x16, 2, 15 + 0 },  { MT_INTRA16x16, 3, 15 + 0 },  { MT_INTRA16x16, 0, 15 + 16 },
    { MT_INTRA16x16, 1, 15 + 16 }, { MT_INTRA16x16, 2, 15 + 16 }, { MT_INTRA16x16, 3, 15 + 16 },
    { MT_INTRA16x16, 0, 15 + 32 }, { MT_INTRA16x16, 1, 15 + 32 }, { MT_INTRA16x16, 2, 15 + 32 },
    { MT_INTRA16x16, 3, 15 + 32 }, { MT_INTRA_PCM, -1, -1 },
};

/* try template for static dispatch */
FORCE_INLINE int decode_mb_type_cabac(Macroblock_t* curMB)
{
    // $spec Table[9-34] - Syntax elements and associated types of binarization, maxBinIdxCtx, and ctxIdxOffset
    // $spec Table[9-39] - the assignment of ctxIdx increments (ctxIdxInc) to binIdx for all ctxIdxOffset
    // $spec Table[9-36] - Binarization for macroblock types in I slices
    // $spec Chp[9.3.3.1.1.3] - Derivation process of ctxIdxInc for the syntax element mb_type

    Slice_t* slice = curMB->slice;

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

int32_t DecodeBlock_I_CABAC(Macroblock_t* curMB)
{
    int raw_mb_type = decode_mb_type_cabac(curMB);
    curMB->i_mb_info = ff_h264_i_MT_info[raw_mb_type];



    return 0;
}
