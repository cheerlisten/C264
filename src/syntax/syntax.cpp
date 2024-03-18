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

int Parser::SyntaxParse(const BufferView* nalBufView)
{
    // flushing
    if (!nalBufView)
        return 0;

    std::shared_ptr<RefNalu> refNal = makeRefNal(*nalBufView);
    RBSPCursor               cursor(refNal);

    int     result = 0;
    nalu_t* nalu = nullptr;
    if (!(nalu = ParseNAL(cursor)))
        FailedParse("nalu", -1);

    // j264 counting bits from RBSP, instead of nal header
    cursor.BaseShiftBytes(cursor.bit_pos / 8);
    nalu->startcodeprefix_len = refNal->start_code_length;

    switch (nalu->nal_unit_type)
    {
    case NALT::SPS:
    {
        std::unique_ptr<seq_parameter_set_rbsp_t> sps = nullptr;
        if (!(sps = ParseSPS(cursor)))
            FailedParse("SPS", -1);
        this->sps.push_back(std::move(sps));
        sema->EmitSPS(nalu, this->sps.back());
        break;
    }
    case NALT::PPS:
    {
        std::unique_ptr<pic_parameter_set_rbsp_t> pps = nullptr;
        if (!(pps = ParsePPS(cursor, this)))
            FailedParse("SPS", -1);
        this->pps.push_back(std::move(pps));
        sema->EmitPPS(nalu, this->pps.back());
        break;
    }
    case NALT::IDR:
    {
        std::unique_ptr<Slice_t> slice = nullptr;
        if (!(slice = ParseSlice(cursor, this, nalu)))
            FailedParse("Slice", -1);
        break;
    }

    default:
        break;
    }
    // check if whole SODB is consumed
    // AASSERT(cursor.bit_length - cursor.bit_pos < 8);
    return 0;
}

GetBitContext RBSPCursor::GetGBC()
{
    GetBitContext ret;
    if (buf)
    {
        ret.buffer = buf;
        ret.buffer_end = buf + bit_length / 8;
        ret.index = bit_pos;
        ret.size_in_bits = bit_length;
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