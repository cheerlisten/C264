#pragma once
#include <c264/c264.h>

class CDecoder: public IDecoder{
    public:
    virtual int Initialize(const DecoderSetting* setting) override;
    virtual int UnInitialize() override;
    virtual int SendPacket(const BufferView *nalBufView) override;
    virtual int GetFrame(const Picture** out_pic) override;
};
