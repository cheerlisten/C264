#pragma once
#include <stdint.h>
#include <tables.h>

struct ContextVariable
{
    int32_t pStateIdx;
    int32_t valMPS;
};

void InitContextVariables(int cabac_init_idc, int slice_qp, ContextVariable outCtxVars[CABAC_CONTEXT_CNT]);

struct BACDecoder
{
    const uint8_t* buffer;
    const uint8_t* buffer_cur;
    const uint8_t* buffer_end;

    uint64_t codIOffset;
    uint64_t codIRange;
    int      leftBits;

    void     Init(const uint8_t* _buffer, int32_t _size);
    uint32_t DecodeDecision(ContextVariable* ctx);
    int      DecodeByPass();
    int      DecodeTermination();

  private:
    uint32_t ReadBytes(int32_t nLessThan5);
};

enum CABAC_SE
{
    /* slice_data */
    SE_MB_SKIP_FLAG,
    SE_MB_FIELD_DECODING_FLAG,

    /* macroblock_layer */
    SE_MB_TYPE,
    SE_TRANSFORM_SIZE_8X8_FLAG,
    SE_CODED_BLOCK_PATTERN_LUMA,
    SE_CODED_BLOCK_PATTERN_CHROMA,
    SE_MB_QP_DELTA,

    /* mb_pred */
    SE_PREV_INTRA4X4_PRED_MODE_FLAG,
    SE_REM_INTRA4X4_PRED_MODE,
    SE_PREV_INTRA8X8_PRED_MODE_FLAG,
    SE_REM_INTRA8X8_PRED_MODE,
    SE_INTRA_CHROMA_PRED_MODE,

    /* mb_pred() + sub_mb_pred */
    SE_REF_IDX_10,
    SE_REF_IDX_11,
    SE_MVD_10_XX0,
    SE_MVD_11_XX0,
    SE_MVD_10_XX1,
    SE_MVD_11_XX1,

    /* sub_mb_pred */
    SE_SUB_MB_TYPE,

    /* residual_block_cabac */
    SE_CODED_BLOCK_FLAG,
    SE_SIGNIFICANT,
    COEFF_FLAG,
    SE_LAST_SIGNIFICANT_COEFF_FLAG,
    SE_COEFF_ABS_LEVEL_MINUS1,
    SE_CNT
};
struct Macroblock_t;

#define CABAC_DecodeDecision(ctxInc) slice->cabac.decoder.DecodeDecision(slice->cabac.bac_contexts + ctxInc)
#define CABAC_DecodeByPass() slice->cabac.decoder.DecodeByPass()
#define CABAC_DecodeTermination() slice->cabac.decoder.DecodeTermination()
