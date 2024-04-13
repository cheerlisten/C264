#include <string.h>
#include <syntax/syntax.h>

static ContextVariable g_kAllContextVariables[CABAC_INIT_IDC_CNT][QP_CNT + 1][CABAC_CONTEXT_CNT];

static void InitAllContextVariables()
{
    for (int32_t iModel = 0; iModel < 4; iModel++)
    {
        const int8_t(*cabac_context_init)[CABAC_CONTEXT_CNT][2] =
            iModel == 0 ? &cabac_context_init_I : &cabac_context_init_PB[iModel];
        for (int32_t iQp = 0; iQp <= QP_CNT; iQp++)
            for (int32_t iIdx = 0; iIdx < CABAC_CONTEXT_CNT; iIdx++)
            {
                int32_t m = (*cabac_context_init)[iIdx][0];
                int32_t n = (*cabac_context_init)[iIdx][1];
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
                g_kAllContextVariables[iModel][iQp][iIdx].pStateIdx = pStateIdx;
                g_kAllContextVariables[iModel][iQp][iIdx].valMPS = valMPS;
            }
    }
}

void InitContextVariables(int cabac_init_idc, int slice_qp, ContextVariable outCtxVars[CABAC_CONTEXT_CNT])
{
    static int _ = [&]() {
        InitAllContextVariables();
        return 0;
    }();
    memcpy(outCtxVars, g_kAllContextVariables[cabac_init_idc][slice_qp], sizeof(ContextVariable) * CABAC_CONTEXT_CNT);
}

uint32_t BACDecoder::ReadBytes(int32_t nLessThan5)
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

void BACDecoder::Init(const uint8_t* _buffer, int32_t _size)
{
    buffer = _buffer;
    buffer_cur = _buffer;
    buffer_end = _buffer + _size;

    codIOffset = ReadBytes(4);
    codIOffset = (codIOffset << 8) | ReadBytes(1);
    leftBits = 31;
    codIRange = 510;
}

uint32_t BACDecoder::DecodeDecision(ContextVariable* ctx)
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

int BACDecoder::DecodeByPass()
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

int BACDecoder::DecodeTermination()
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