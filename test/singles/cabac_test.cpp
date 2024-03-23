#include <cabac_test.h>
#include <string>
struct ContextVariable
{
    int32_t pStateIdx;
    int32_t valMPS;
};

ContextVariable AllContextVariables[CABAC_INIT_IDC_CNT][QP_CNT + 1][CABAC_CONTEXT_CNT];

static void InitAllContextVariables()
{
    for (int32_t iModel = 0; iModel < 4; iModel++)
    {
        for (int32_t iQp = 0; iQp <= QP_CNT; iQp++)
            for (int32_t iIdx = 0; iIdx < CABAC_CONTEXT_CNT; iIdx++)
            {
                int32_t m = g_kiCabacGlobalContextIdx[iIdx][iModel][0];
                int32_t n = g_kiCabacGlobalContextIdx[iIdx][iModel][1];
                int32_t iPreCtxState = WELS_CLIP3((((m * iQp) >> 4) + n), 1, 126);
                uint8_t valMPS = 0;
                uint8_t pStateIdx = 0;
                if (iPreCtxState <= 63)
                {
                    pStateIdx = 63 - iPreCtxState;
                    valMPS = 0;
                }
                else
                {
                    pStateIdx = iPreCtxState - 64;
                    valMPS = 1;
                }
                AllContextVariables[iModel][iQp][iIdx].pStateIdx = pStateIdx;
                AllContextVariables[iModel][iQp][iIdx].valMPS = valMPS;
            }
    }
}

template<typename CacheType = uint64_t>
struct BACDecoder
{
    const uint8_t* buffer;
    const uint8_t* buffer_cur;
    const uint8_t* buffer_end;

    CacheType codIOffset;
    CacheType codIRange;
    int       leftBits;

    uint32_t ReadBytes(int32_t nLessThan5)
    {
        int leaveBytes = buffer_end - buffer_cur;
        if (nLessThan5 > leaveBytes) //not likely
        {
            nLessThan5 = leaveBytes;
        }
        if (nLessThan5 < 1)
            throw "no more left Bytes";

        uint32_t ret;
        switch (nLessThan5)
        {
        case 1:
            ret = buffer_cur[0];
            buffer_cur += 1;
            break;
        case 2:
            ret = (buffer_cur[0] << 8) | buffer_cur[1];
            buffer_cur += 2;
            break;
        case 3:
            ret = (buffer_cur[0] << 16) | (buffer_cur[1] << 8) | buffer_cur[2];
            buffer_cur += 3;
            break;
        case 4:
            ret = (uint32_t(buffer_cur[0]) << 24) | (buffer_cur[1] << 16) | (buffer_cur[2] << 8) | buffer_cur[3];
            buffer_cur += 4;
            break;
        default:
            throw "out of range";
            break;
        }
        return ret;
    }

    void Init(const uint8_t* _buffer, int32_t _size)
    {
        buffer = _buffer;
        buffer_cur = _buffer;
        buffer_end = _buffer + _size;

        codIOffset = ReadBytes(4);
        codIOffset = (codIOffset << 8) | ReadBytes(1);
        leftBits = 31;
        codIRange = 510;
    }

    uint32_t DecodeDecision(ContextVariable* ctx)
    {
        int qCodIRangeIdx = (codIRange >> 6) & 3;
        int codIRangeLPS = g_kuiCabacRangeLps[ctx->pStateIdx][qCodIRangeIdx];
        int binVal = ctx->valMPS;
        codIRange = codIRange - codIRangeLPS;

        int renorm_btis = 1; //

        if (codIOffset >= (codIRange << leftBits)) // LPS
        {
            codIOffset -= (codIRange << leftBits);
            codIRange = codIRangeLPS;
            binVal = 1 - binVal;

            // State transition process LPS
            if (ctx->pStateIdx == 0)
                ctx->valMPS = 1 - ctx->valMPS;
            ctx->pStateIdx = g_kuiStateTransTable[ctx->pStateIdx][0];

            renorm_btis = g_kRenormTable256[codIRange];
            codIRange <<= renorm_btis;
        }
        else //MPS
        {
            // State transition process MPS

            ctx->pStateIdx = g_kuiStateTransTable[qCodIRangeIdx][1];

            if (codIRange >= 256)
                return binVal;
            else
                codIRange <<= 1; // renorm_btis=1 is deduced
        }

        if (leftBits >= renorm_btis)
            leftBits -= renorm_btis;
        else
        {
            int readedBytes = ReadBytes(4);
            leftBits += readedBytes * 8 - renorm_btis;
        }
        return binVal;
    }

    int DecodeByPass()
    {
        if (leftBits < 1)
            leftBits += ReadBytes(4) * 8;
        leftBits = leftBits - 1;
        if (codIOffset >= (codIRange << leftBits))
        {
            codIOffset -= codIRange;
            return 1;
        }
        else
            return 0;
    }

    int DecodeTermination()
    {
        codIRange -= 2;
        int binVal;

        if (codIOffset >= (codIRange << leftBits))
            binVal = 1;
        else
        {
            binVal = 0;
            if (codIRange < 256)
            {
                int renorm_bits = g_kRenormTable256[codIRange];
                codIRange <<= renorm_bits;
                leftBits -= renorm_bits;
                if (leftBits < 0)
                    leftBits += 8 * ReadBytes(4);
            }
        }
        return binVal;
    }
};

// Table 9-11 the ctxIdx for which initialization is needed for each of the slice types are listed
enum SE
{
    MB_TYPE,
    SE_CNT
};
enum SliceType
{
    SI,
    I,
    P_SP,
    B,
    SLiceTypeCnt
};
constexpr int CtxIdxRange[SE_CNT][SLiceTypeCnt][2] = {
    /* [MB_TYPE] = */ { { 0, 10 }, { 3, 10 }, { 14, 20 }, { 27, 35 } }
};
/* test settings */
uint8_t slice_header_buf[] = { 254, 199, 111, 230, 89, 85, 65, 157 };

SliceType slice_type = I;
int       cabac_init_idc = 0;
int       slice_qp = 15;

int main()
{
    InitAllContextVariables();
    ContextVariable ContextVariables[CABAC_CONTEXT_CNT];
    memcpy(ContextVariables, AllContextVariables[cabac_init_idc][slice_qp], sizeof(ContextVariables));
    for (int i = 0; i < 63; ++i)
        for (int j = 0; j < 4; ++j)
        {
            int lps = g_kuiCabacRangeLps[i][j];
            int renor_btis = g_kRenormTable256[lps];
            int slps = lps << renor_btis;
            if (slps < 256)
                return 0;
        }
    //mb_type
    BACDecoder bac;
    bac.Init(slice_header_buf, sizeof(slice_header_buf));

    int mb_type = 0;

    ContextVariable* baseCtx = ContextVariables + CtxIdxRange[MB_TYPE][slice_type][0];

    // $spec Chp 9.3.2.5 Binarization process for macroblock type and sub-macroblock type

    // $spec Table 9-39:
    int condTermFlagA = 0, condTermFlagB = 0;
    int ctxInc0 = condTermFlagA + condTermFlagB;

    int b0 = bac.DecodeDecision(baseCtx + ctxInc0);
    if (b0 == 0)
        mb_type = 0; // I_NxN
    else
    {
        int b1 = bac.DecodeTermination();
        if (b1 == 1)
            mb_type = 25; //I_PCM
        else              // I_16x16
        {
            int b2 = bac.DecodeDecision(baseCtx + 3);
            int b3 = bac.DecodeDecision(baseCtx + 4);
            mb_type = 1 + b2 * 12 + b3 * 4;
            if (b3)
            {
                int bmore = bac.DecodeDecision(baseCtx + 5);
                mb_type += bmore * 4;
            }
            int bb4 = bac.DecodeDecision(baseCtx + 6);
            int bb5 = bac.DecodeDecision(baseCtx + 7);
            mb_type += bb4 * 2 + bb5;
        }
    }

    return 0;
}
