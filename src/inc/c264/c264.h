#pragma once
#include <c264/config.h>
#include <cstdint>

#define MAX_CHANNEL 3

struct DecoderSetting
{
};

struct BufferView
{
    uint8_t* buf;
    uint32_t size;
    /* enhanced debug info, leave zero-filled */
    uint64_t source_pos;
};

struct Picture
{
    uint8_t* data[MAX_CHANNEL];
    uint32_t strides[MAX_CHANNEL];
    uint32_t width, height;
};

class IDecoder
{
  public:
    virtual int Initialize(const DecoderSetting* setting) = 0;
    virtual int UnInitialize() = 0;
    /// @return 0: OK,  -1: need retreive frames first, other: ... 
    virtual int SendPacket(const BufferView* nalBufView) = 0;
    /// @return 0: OK,  -1: need more input, other: ... 
    virtual int GetFrame(const Picture** out_pic) = 0;
};

extern "C" API int CreateDecoder(IDecoder** ppDecoder);
extern "C" API int DestroryDecoder(IDecoder** ppDecoder);
