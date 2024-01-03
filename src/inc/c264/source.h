#pragma once
#include <cstdint>
#include <string>

#include <c264/c264.h>

enum SourceType{
    E_SOURCE_ANNEXB = 0x1,
};

struct SourceSetting
{
    SourceType type;
    std::string url;
};

class ISource
{
  public: 
    virtual int Initialize(const SourceSetting* setting) = 0;
    virtual int UnInitialize() = 0;
    /// @brief read next nal
    /// @param bufview 
    /// @return 0:SUCCESS -1: reach EOF
    virtual int NextNAL(BufferView* bufview) = 0;
    virtual const SourceSetting* GetSetting() = 0;
};

#ifdef CONFIG_SOURCE
extern "C" API int CreateNALSource(ISource** ppSource);
extern "C" API int DestroryNALSource(ISource** ppSource);
#endif
