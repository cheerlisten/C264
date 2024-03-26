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