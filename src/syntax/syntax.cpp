#include <memory>
#include <syntax/syntax.h>
#include <sema/sema.h>
#include <log.h>
#define FailedParse(elementDesc, retval)                                                                               \
    do                                                                                                                 \
    {                                                                                                                  \
        LOG(LL_Warn, elementDesc " parse failed, source_pos=%d", nalBufView->source_pos);                              \
        return retval;                                                                                                 \
    } while (0)

int SyntaxParse(const BufferView* nalBufView, struct Sema* sema)
{
    std::shared_ptr<RefNalu> refNal = makeRefNal(*nalBufView);
    RBSPCursor               cursor(refNal);

    int     result = 0;
    nalu_t* nalu = nullptr;
    if (!(nalu = ParseNAL(cursor)))
        FailedParse("nalu", -1);

    nalu->startcodeprefix_len = refNal->start_code_length;

    switch (nalu->nal_unit_type)
    {
    case NALT::SPS:
    {
        seq_parameter_set_rbsp_t* sps = nullptr;
        if (!(sps = ParseSPS(cursor)))
            FailedParse("SPS", -1);
        sema->EmitSPS(nalu, sps);
        break;
    }
    case NALT::PPS:
    {
        break;
    }

    default:
        break;
    }
    return 0;
}

GetBitContext RBSPCursor::GetGBC()
{
    GetBitContext ret;
    if (buf)
    {
        ret.buffer = buf;
        ret.buffer_end = buf + size;
        ret.index = bit_pos;
        ret.size_in_bits = size * 8;
    }
    return ret;
}

std::shared_ptr<RefNalu> makeRefNal(BufferView bufView)
{
    auto refNal = std::make_shared<RefNalu>();
    {
        auto p = bufView.buf;
        while (*p++ != 0x01)
            ;
        refNal->start_code_length = p - bufView.buf;
        refNal->nalu_len = bufView.size - refNal->start_code_length;
        refNal->source_pos = bufView.source_pos;
    }
    refNal->data = std::vector<uint8_t>(bufView.buf + refNal->start_code_length, bufView.buf + bufView.size);
    int rbsp_end = EBSP2RBSP_Inplace(refNal->data.data(), 0, refNal->data.size());
    refNal->data.resize(rbsp_end);
    return refNal;
};