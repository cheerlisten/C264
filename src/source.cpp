#include <stdio.h>
#include <string>
#include <vector>

#include <c264/source.h>
#include <log.h>

const static uint32_t BUFFER_CHUNCK_BASE_SIZE = 4096 * 2;

class CFileSource : public ISource
{
  public:
    CFileSource();
    ~CFileSource();
    virtual int Initialize(const SourceSetting* setting) override;
    virtual int UnInitialize() override;
    virtual int NextNAL(BufferView* bufview) override;
    virtual const SourceSetting* GetSetting() override;

  private:
    // return seeked pos
    uint64_t SeekTo(uint64_t pos);
    // return incred buffer size
    uint64_t IncBuffer(uint64_t more_size);
    uint64_t FoundNextStartCode(uint64_t start_ptr);
    inline uint64_t BufferSize()
    {
        return m_buffer_end - m_buffer;
    }

  private:
    SourceSetting m_setting;
    FILE* m_file = nullptr;
    uint64_t m_file_totalsize = 0;
    /*
    **********************************************************************************
    *                            |              buffer_size              |
    *                            |---------------------------------------|
    *                            |                                       |
    *
    *                         buffer          buf_ptr       buf_end
    *                            +---------------+-----------------------+
    *                            |/ / / / / / / /|/ / / / / / /|         |
    *  read buffer:              |/ / consumed / | to be read /|         |
    *                            |/ / / / / / / /|/ / / / / / /|         |
    *                            +---------------+-----------------------+
    *
    *                                                         pos
    *              +-------------------------------------------+-----------------+
    *  input file: |                                           |                 |
    *              +-------------------------------------------+-----------------+
    *              |                                                             |
    **********************************************************************************
    */
    uint64_t m_buffer_pos = 0;
    uint8_t* m_buffer = nullptr;
    // using relative ptr to reduce one relation between m_buffer* with _m_buffer
    uint64_t m_buffer_ptr = 0;
    uint8_t* m_buffer_end = nullptr;
    std::vector<uint8_t> _m_buffer;
    std::vector<uint8_t> m_outputBuffer;
};

CFileSource::CFileSource()
{
    memset(&m_setting, 0, sizeof(m_setting));
}
CFileSource::~CFileSource()
{
    UnInitialize();
}

int CFileSource::Initialize(const SourceSetting* setting)
{
    m_file = fopen(setting->url.c_str(), "rb");
    if (!m_file || setting->type != E_SOURCE_ANNEXB)
        return -1;

    fseek(m_file, 0, SEEK_END);
    m_file_totalsize = ftell(m_file);
    rewind(m_file);

    _m_buffer.reserve(std::min<uint32_t>(BUFFER_CHUNCK_BASE_SIZE, m_file_totalsize));
    m_setting = *setting;
    auto nextCode = FoundNextStartCode(0);
    if (nextCode == -1ull)
        return -2;
    SeekTo(nextCode + m_buffer_pos - BufferSize());
    return 0;
}

int CFileSource::UnInitialize()
{
    if (m_file)
    {
        fclose(m_file);
        m_file = 0;
    }
    return 0;
}

int CFileSource::NextNAL(BufferView* bufview)
{
    int nextNalFounded = 0;
    uint64_t nextCode = FoundNextStartCode(m_buffer_ptr + 2);
    if (nextCode == -1ull && m_buffer_ptr != BufferSize())
        nextCode = m_buffer_pos;
    if (nextCode == -1ull)
        return -1;
    else
    {
        bufview->size = nextCode - m_buffer_ptr;
        m_outputBuffer.resize(bufview->size);
        memcpy(m_outputBuffer.data(), m_buffer + m_buffer_ptr, bufview->size);
        bufview->buf = m_outputBuffer.data();
        bufview->source_pos = (m_buffer_pos - BufferSize() + m_buffer_ptr);
        SeekTo(bufview->source_pos + bufview->size);
    }
    return 0;
}

const SourceSetting* CFileSource::GetSetting()
{
    return &m_setting;
}

uint64_t CFileSource::SeekTo(uint64_t pos)
{
    if (m_buffer_pos - BufferSize() <= pos && pos < m_buffer_pos)
    {
        m_buffer_ptr = pos - (m_buffer_pos - BufferSize());
        return pos;
    }
    else if (pos == m_file_totalsize)
    {
        m_buffer_ptr = BufferSize();
        return pos;
    }
}

uint64_t CFileSource::IncBuffer(uint64_t more_size)
{
    more_size = std::min(m_file_totalsize - m_buffer_pos, more_size);
    _m_buffer.resize(_m_buffer.size() + more_size);
    auto buf = _m_buffer.data() + _m_buffer.size() - more_size;

    int read_size = fread(buf, 1, more_size, m_file);

    if (read_size != more_size)
        _m_buffer.resize(_m_buffer.size() - more_size + read_size);
    m_buffer_pos = ftell(m_file);
    m_buffer = _m_buffer.data();
    m_buffer_end = m_buffer + _m_buffer.size();

    if (!read_size)
        LOG("CFileSource::IncBuffer reach end");
    return read_size;
}

uint64_t CFileSource::FoundNextStartCode(uint64_t start_ptr)
{
    int zeroEncountered = 0;
    uint64_t bufPtr = start_ptr;
    while (1)
    {
        uint8_t* const& buf = m_buffer;
        uint64_t bufSize = BufferSize();

        for (; bufPtr + 3 < bufSize; ++bufPtr)
        {
            if (buf[bufPtr] == 0x00)
            {
                ++zeroEncountered;
                if (zeroEncountered == 2)
                    if (buf[bufPtr + 1] == 0x01 || buf[bufPtr + 1] == 0x00 && buf[bufPtr + 2] == 0x01)
                        return bufPtr - (zeroEncountered - 1);
            }
            else
                zeroEncountered = 0;
        }

        int inc_size = IncBuffer(BUFFER_CHUNCK_BASE_SIZE);
        if (inc_size)
            continue;
        else
        {
            if (bufPtr == bufSize - 3 && buf[bufSize - 3] == 0x00 && buf[bufSize - 2] == 0x00 && buf[bufPtr - 1] == 0x01)
                return bufSize - 3;
            else
                return -1ull;
        }
    }
}

#ifdef CONFIG_SOURCE

extern "C" API int CreateNALSource(ISource** ppSource)
{
    if (!ppSource)
        return -1;
    *ppSource = new CFileSource;
    return 0;
}
extern "C" API int DestroryNALSource(ISource** ppSource)
{
    if (!ppSource)
        return -1;
    delete (CFileSource*)ppSource;
    return 0;
}
#endif