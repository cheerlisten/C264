#pragma once
#include <assert.h>
#include <stdint.h>
#include <vector>

#include <c264/source.h>

class AnnexReader : public ISource
{
    using byte = uint8_t;

  public:
    AnnexReader();
    ~AnnexReader();
    virtual int Initialize(const SourceSetting* setting) override;
    virtual int UnInitialize() override;
    /// @brief get next nalu
    /// @return 0:OK
    virtual int                  NextNAL(BufferView* bufView) override;
    virtual const SourceSetting* GetSetting() override;
    enum
    {
        IOBUF_PADDING = 8
    };

  private:
    // 0:OK 1:reach EOF
    int AdvanceToNextStartCode();
    struct IoBufReader
    {
        uint32_t buf_idx = 0;
        uint32_t buf_sz = 0;
        bool     reach_EOF = false;
        byte*    iobuf = nullptr;
        uint32_t iobuf_sz = 4096;
        uint32_t iobuf_pos = 0;
        FILE*    fd = nullptr;
        int      NextBuffer()
        {
            buf_sz = (uint32_t)fread(iobuf, 1, iobuf_sz, fd);
            buf_idx = 0;
            reach_EOF = buf_sz == 0;
            iobuf_pos += buf_sz;
            return buf_sz;
        }
    };
    struct OutBuf
    {
        std::vector<byte> buf;
        uint32_t          buf_idx = 0;
        uint32_t          buf_end_idx = 0;
        int               IncBuffer(uint32_t more_sz)
        {
            uint32_t new_sz = (uint32_t)buf.size() + more_sz;
            buf.resize(new_sz);
            return 0;
        }
    };
    struct AuxInfo
    {
        uint32_t readedNextStartCodeLength = 0;
        byte     leadingZeroCount = 0;
        bool     firstOneOut = true;
    };

    SourceSetting m_setting;

    IoBufReader io;
    OutBuf      out;
    AuxInfo     aux;
};