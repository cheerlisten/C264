#include <decoder.h>

int CDecoder::Initialize(const DecoderSetting* setting)
{
    parser.sema = &sema;
    return 0;
}

int CDecoder::UnInitialize()
{
    return 0;
}

int CDecoder::SendPacket(const BufferView* nalBufView)
{
    AASSERT(parser.SyntaxParse(nalBufView) == 0);
    return 0;
}

int CDecoder::GetFrame(const Picture** out_pic)
{
    return -1;
}
