#include <stdarg.h>
#include <stdio.h>

#include <c264/c264.h>
#include <c264/source.h>
#include <decoder.h>
#include <log.h>

struct MemoryAllocator_Default
{
    uint8_t* Alloca(uint32_t size)
    {
        return new uint8_t[size];
    }
    void DeAlloca(const uint8_t* buffer, uint32_t size)
    {
        delete[] buffer;
    }
    static MemoryAllocator* Instance()
    {
        static MemoryAllocator _ins = [] {
            auto            fnAlloc = &MemoryAllocator_Default::Alloca;
            auto            fnDeAlloca = &MemoryAllocator_Default::DeAlloca;
            MemoryAllocator m;
            m.alloc = *decltype(&m.alloc)(&fnAlloc);
            m.dealloc = *decltype(&m.dealloc)(&fnDeAlloca);
            m.opaque = nullptr;
            return m;
        }();
        return &_ins;
    }
};

extern "C" API uint8_t* AllocMemory(const uint32_t size, MemoryAllocator* allocator)
{
    if (!allocator)
        allocator = MemoryAllocator_Default::Instance();
    return allocator->alloc(allocator, size);
}

extern "C" API void FreeMemory(const uint8_t* buffer, int size, MemoryAllocator* allocator)
{
    if (!allocator)
        allocator = MemoryAllocator_Default::Instance();
    allocator->dealloc(allocator, buffer, size);
}

extern "C" API int CreateDecoder(IDecoder** ppDecoder)
{
    if (ppDecoder == 0)
        return -1;
    *ppDecoder = new CDecoder;
    return 0;
}

extern "C" API int DestroryDecoder(IDecoder** ppDecoder)
{
    if (ppDecoder == 0)
        return -1;
    delete (CDecoder*)*ppDecoder;
    *ppDecoder = nullptr;
    return 0;
}

void Log(const char* fmt, ...)
{
    char    buf[1024];
    
    va_list ls;
    va_start(ls, fmt);
    int msgLen = vsnprintf(buf, 1024, fmt, ls);
    buf[msgLen++] = '\n';
    buf[msgLen] = '\0';

    fprintf(stdout, buf);
}