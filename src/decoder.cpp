#include <decoder.h>

int CDecoder::Initialize(const DecoderSetting* setting)
{
    return 0;
}

int CDecoder::UnInitialize()
{
    return 0;
}

int CDecoder::SendPacket(const BufferView* nalBufView)
{
    return -1;
}

int CDecoder::GetFrame(const Picture** out_pic)
{
    return -1;
}
