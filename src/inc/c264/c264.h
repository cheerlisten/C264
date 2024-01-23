#pragma once
#include <stdint.h>
#include <c264/config.h>

#define MAX_CHANNEL 3

struct MemoryAllocator
{
    uint8_t* (*alloc)(void* opaque, uint32_t size);
    void (*dealloc)(void* opaque, const uint8_t* buf, uint32_t size);

    /* in case of state-aware allocatator */
    void* opaque;
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

struct DecoderSetting
{
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

/// @brief Allocate a buffer with size=${size}, from allocator
/// @param allocator leave zero for default allocator
/// @return non-null:OK
extern "C" API uint8_t* AllocMemory(const uint32_t size, MemoryAllocator* allocator = nullptr);
extern "C" API void     FreeMemory(const uint8_t* buffer, int size, MemoryAllocator* allocator = nullptr);

extern "C" API int CreateDecoder(IDecoder** ppDecoder);
extern "C" API int DestroryDecoder(IDecoder** ppDecoder);
