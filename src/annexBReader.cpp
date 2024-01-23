#include <annexbReader.h>

int AnnexReader::Initialize(const SourceSetting* setting)
{
    io.fd = fopen(setting->url.c_str(), "rb");
    if (!io.fd)
        return -1;
    io.iobuf = new byte[io.iobuf_sz + IOBUF_PADDING];
    m_setting = *setting;
    return 0;
}

int AnnexReader::UnInitialize()
{
    if (io.fd)
        fclose(io.fd);
    if (io.iobuf)
        delete[] io.iobuf;
    io = IoBufReader();
    out = OutBuf();
    aux = AuxInfo();
    return 0;
}

int AnnexReader::NextNAL(BufferView* bufView)
{
    if (aux.firstOneOut)
    {
        aux.firstOneOut = false;
        int r1 = AdvanceToNextStartCode();
    }

    uint32_t aux_pos = io.iobuf_pos - io.buf_sz + io.buf_idx - aux.leadingZeroCount - (aux.readedNextStartCodeLength);
    AdvanceToNextStartCode();

    if (out.buf_end_idx > 0)
    {
        bufView->buf = out.buf.data() + out.buf_idx;
        bufView->size = out.buf_end_idx - out.buf_idx;
        bufView->source_pos = aux_pos;
        return 0;
    }
    else
        return -1;
}

int AnnexReader::AdvanceToNextStartCode()
{
    out.buf_idx = 0;
    out.buf_end_idx = 0;

    int zero_count = 0;
#define idx io.buf_idx
    uint32_t idx_start = idx;

    auto CopyByPassBuffer = [&/*out, ifCopyByPassBuffer*/](byte* data, uint32_t sz) {
        if (!sz)
            return;
        if (out.buf_end_idx + sz > out.buf.size())
            out.IncBuffer(out.buf_end_idx + sz - (uint32_t)out.buf.size());
        std::memcpy(out.buf.data() + out.buf_end_idx, data, sz);
        out.buf_end_idx += sz;
    };
    auto SetEnd = [&] {
        aux.leadingZeroCount = zero_count > 3 ? zero_count - 3 : 0;
        aux.readedNextStartCodeLength = zero_count == 2 ? 3 : 4;
        if (idx_start + zero_count < idx)
            CopyByPassBuffer(io.iobuf + idx_start, idx - idx_start - zero_count);
        else // zeros stay in copyed buffer
            out.buf_end_idx -= (idx_start + 2 - idx);
        idx++; // skip 0x01
    };

    if (aux.readedNextStartCodeLength)
    {
        out.buf.resize(aux.leadingZeroCount + aux.readedNextStartCodeLength);
        std::memset(out.buf.data(), 0, out.buf.size());
        out.buf.back() = 0x01;
        out.buf_end_idx = out.buf.size();
        aux.readedNextStartCodeLength = 0;
        aux.leadingZeroCount = 0;
    }

    while (1)
    {
        if (io.buf_idx >= io.buf_sz)
        {
            CopyByPassBuffer(io.iobuf + idx_start, io.buf_idx - idx_start);
            if (io.reach_EOF || !io.NextBuffer())
            {
                break;
            }
            idx_start = io.buf_idx; // maybe a new buffer
        }

        // check if this is the end of start_code
        if (zero_count > 1 && io.iobuf[io.buf_idx] == 0x01)
        {
            SetEnd();
            goto founded;
        }

        // to avoid boundary checking
        if (io.buf_sz > 1)
        {
            uint32_t buf_sz_minus1 = io.buf_sz - 1;
            for (; idx < buf_sz_minus1;)
            {
                if (io.iobuf[idx] != 0x00)
                    zero_count = 0;
                else
                {
                    ++zero_count;
                    if (zero_count > 1 && io.iobuf[idx + 1] == 0x01)
                    {
                        idx++;
                        SetEnd();
                        goto founded;
                    }
                }
                ++idx;
#if 1 // enable fast forward
                {
                    int idx1 = idx;
    #if HAVE_FAST64 // fast64
                    while (idx1 < io.buf_sz)
                    {
                        uint64_t u64 = *(uint64_t*)(io.iobuf + idx1);
                        if (~u64 & (u64 - 0x0101010101010101ull) & 0x8080808080808080ull)
                            break;
                        else
                            idx1 += 8;
                    }
    #else
                    while (idx1 < io.buf_sz)
                    {
                        uint32_t u32 = *(uint32_t*)(io.iobuf + idx1);
                        if (~u32 & (u32 - 0x01010101u) & 0x80808080u)
                            break;
                        else
                            idx1 += 4;
                    }
    #endif

                    if (idx1 != idx)
                    {
                        zero_count = 0;
                        idx = idx1;
                    }
                }
#endif
            }
        }

        if (io.iobuf[idx] != 0x00)
            zero_count = 0;
        else
            zero_count++;
        idx++;
    }
    // eof
    assert(io.reach_EOF);
    return -1;
founded:
    return 0;
}

const SourceSetting* AnnexReader::GetSetting()
{
    return &m_setting;
}

AnnexReader::AnnexReader()
{
    memset(&m_setting, 0, sizeof(m_setting));
}

AnnexReader::~AnnexReader()
{
    UnInitialize();
}
