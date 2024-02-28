#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <chrono>
#include <functional>
#include <vector>
using byte = uint8_t;
struct BufferView
{
    byte* buf;
    uint32_t sz;
    uint32_t file_pos;
};
#define LOG(...) printf(__VA_ARGS__)
// #define LOG(...)
struct AnnexReader
{
    int Init(const char* file, uint32_t io_buf_sz = 0);
    int UnInit();
    /// @brief get next nalu
    /// @return 0:OK
    int NextNALU(BufferView* bufView);
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
        bool reach_EOF = false;
        byte* iobuf = nullptr;
        uint32_t iobuf_sz = 4096;
        uint32_t iobuf_pos = 0;
        FILE* fd = nullptr;
        int NextBuffer()
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
        uint32_t buf_idx = 0;
        uint32_t buf_end_idx = 0;
        int IncBuffer(uint32_t more_sz)
        {
            uint32_t new_sz = (uint32_t)buf.size() + more_sz;
            buf.resize(new_sz);
            return 0;
        }
    };
    struct AuxInfo
    {
        uint32_t readedNextStartCodeLength = 0;
        byte leadingZeroCount = 0;
        bool firstOneOut = true;
    };

    IoBufReader io;
    OutBuf out;
    AuxInfo aux;
};

int AnnexReader::Init(const char* file, uint32_t io_buf_sz)
{
    io.fd = fopen(file, "rb");
    if (!io.fd)
        return -1;
    io.iobuf = new byte[io.iobuf_sz + IOBUF_PADDING];

    return 0;
}

int AnnexReader::UnInit()
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

int AnnexReader::NextNALU(BufferView* bufView)
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
        bufView->sz = out.buf_end_idx - out.buf_idx;
        bufView->file_pos = aux_pos;
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

struct AtExit
{
    std::function<void()> fn = nullptr;
    AtExit(std::function<void()> fn_) :
        fn(fn_)
    {
    }
    ~AtExit()
    {
        if (engaged && fn)
            fn();
    }
    AtExit(const AtExit&) = delete;
    AtExit(AtExit&& o)
    {
        fn = o.fn;
        o.engaged = false;
    }
    bool engaged = true;
};

AtExit makeExit(std::function<void()> f)
{
    AtExit at(f);
    return at;
}

int main()
{
    auto start = std::chrono::system_clock::now();
    auto atExit = makeExit([&] {
        auto now = std::chrono::system_clock::now();
        printf("cost %lldms\n", std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count());
    });

    for (int i = 0; i < 100; ++i)
    {
        AnnexReader reader;
        reader.Init(CMAKE_SOURCE_DIR "/../C264-Resource/Res/USA.h264");
        // reader.Init(R"(C:\Users\Administrator\Videos\In\USA.h264)");
        BufferView bufView;

        int nalu_count = 0;
        int frame_count = 0;
        while (1)
        {
            int reader_ret = reader.NextNALU(&bufView);
            if (!!reader_ret)
                break;
            int x = 0;
            while (bufView.buf[x++] != 0x01)
                ;
            uint32_t type = bufView.buf[x] & 0x1f;
            nalu_count++;
            frame_count += type <= 5;
            LOG("meet nalu:%4d f:%d type=%2d(%3x)   size=%5d    pos=%d\n",
                nalu_count,
                frame_count,
                type,
                uint32_t(bufView.buf[x]),
                bufView.sz,
                bufView.file_pos);
        }
        reader.UnInit();
    }
}
