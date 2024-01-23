#include <assert.h>
#include <chrono>
#include <cstring>
#include <functional>
#include <memory>
#include <stdio.h>
#include <stdint.h>
#include <vector>
using byte = uint8_t;
using std::vector;
typedef enum
{
    NALU_TYPE_SLICE = 1,
    NALU_TYPE_DPA = 2,
    NALU_TYPE_DPB = 3,
    NALU_TYPE_DPC = 4,
    NALU_TYPE_IDR = 5,
    NALU_TYPE_SEI = 6,
    NALU_TYPE_SPS = 7,
    NALU_TYPE_PPS = 8,
    NALU_TYPE_AUD = 9,
    NALU_TYPE_EOSEQ = 10,
    NALU_TYPE_EOSTREAM = 11,
    NALU_TYPE_FILL = 12,
#if (MVC_EXTENSION_ENABLE)
    NALU_TYPE_PREFIX = 14,
    NALU_TYPE_SUB_SPS = 15,
    NALU_TYPE_SLC_EXT = 20,
    NALU_TYPE_VDRD = 24 // View and Dependency Representation Delimiter NAL Unit
#endif
} NaluType;
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
#define CASE 1 | 0x08
#if CASE // enable_fast
                /* align would slow down */
                // #if CASE & 0x09
                //                 if (!(idx & 0x11))
                // #else
                //                 if (!(idx & 0x111))
                // #endif
                {
                    int idx1 = idx;
    #if CASE & 0x1 // fast32
                    while (idx1 < io.buf_sz)
                    {
                        uint32_t u32 = *(uint32_t*)(io.iobuf + idx1);
                        if (~u32 & (u32 - 0x01010101u) & 0x80808080u)
                            break;
                        else
                            idx1 += 4;
                    }
    #else // fast64
                    while (idx1 < io.buf_sz)
                    {
                        uint64_t u64 = *(uint64_t*)(io.iobuf + idx1);
                        if (~u64 & (u64 - 0x0101010101010101ull) & 0x8080808080808080ull)
                            break;
                        else
                            idx1 += 8;
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

#define ZEROBYTES_SHORTSTARTCODE 2
int EBSPtoRBSP(byte* streamBuffer, int begin_bytepos, int end_bytepos)
{
    int i, j, count;
    count = 0;

    if (end_bytepos < begin_bytepos)
        return end_bytepos;

    j = begin_bytepos;

    for (i = begin_bytepos; i < end_bytepos; ++i)
    { // starting from begin_bytepos to avoid header information
        // in NAL unit, 0x000000, 0x000001 or 0x000002 shall not occur at any byte-aligned position
        if (count == ZEROBYTES_SHORTSTARTCODE && streamBuffer[i] < 0x03)
            return -1;
        if (count == ZEROBYTES_SHORTSTARTCODE && streamBuffer[i] == 0x03)
        {
            // check the 4th byte after 0x000003, except when cabac_zero_word is used, in which case the last three bytes of this NAL unit must be 0x000003
            if ((i < end_bytepos - 1) && (streamBuffer[i + 1] > 0x03))
                return -1;
            // if cabac_zero_word is used, the final byte of this NAL unit(0x03) is discarded, and the last two bytes of RBSP must be 0x0000
            if (i == end_bytepos - 1)
                return j;

            ++i;
            count = 0;
        }
        streamBuffer[j] = streamBuffer[i];
        if (streamBuffer[i] == 0x00)
            ++count;
        else
            count = 0;
        ++j;
    }

    return j;
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

struct RefBuffer
{
    std::vector<byte> data;
};

// RBSP data
struct RefNal : RefBuffer
{
    // debug info
    uint64_t source_pos;
    int32_t nalu_len;
    int start_code_length;
};

struct DataView
{
    DataView(std::shared_ptr<RefNal> refNal_) :
        refNal(refNal_)
    {
        buf = refNal->data.data() + 1;
        size = refNal->data.size() - 1;
    }
    std::shared_ptr<RefNal> refNal;
    byte* buf;
    int32_t size;
};

std::shared_ptr<RefNal> makeRefNal(BufferView bufView)
{
    std::shared_ptr<RefNal> refNal = std::make_shared<RefNal>();
    {
        auto p = bufView.buf;
        while (*p++ != 0x01)
            ;
        refNal->start_code_length = p - bufView.buf;
        refNal->nalu_len = bufView.sz;
        refNal->source_pos = bufView.file_pos;
    }
    refNal->data = std::vector<byte>(bufView.buf + refNal->start_code_length, bufView.buf + bufView.sz);
    int rbsp_end = EBSPtoRBSP(refNal->data.data(), 0, refNal->data.size());
    refNal->data.resize(rbsp_end);
    return refNal;
};

uint32_t Swap32(uint32_t u32)
{
    return (u32 >> 24) | (u32 >> 8 & 0xff00) | (u32 << 24) | (u32 << 8 & 0xff0000);
}
uint32_t get_bits(byte* buffer, int& bitCounter, int n)
{
    assert(((bitCounter & 7) + n) <= 32);
    const uint32_t buffer32 = Swap32(*(const uint32_t*)(buffer + (bitCounter >> 3)));
    int bitOffset = bitCounter & 0x7;
    uint32_t ret = (buffer32) << bitOffset >> (32 - n);
    bitCounter += n;
    return ret;
}
uint32_t show_bits(byte* buffer, int& bitCounter, int n)
{
    assert(((bitCounter & 7) + n) <= 32);
    const uint32_t buffer32 = Swap32(*(const uint32_t*)(buffer + (bitCounter >> 3)));
    int bitOffset = bitCounter & 0x7;
    uint32_t ret = (buffer32) << bitOffset >> (32 - n);
    return ret;
}
void skip_bits(byte* buffer, int& bitCounter, int n)
{
    bitCounter += n;
}
// for the range = [1, 2^13 - 2]
uint32_t read_ue_golomb(byte* buffer, int& bitCounter)
{
    uint32_t byteOffset = bitCounter >> 3;
    uint32_t byteOffset1 = bitCounter & 0x7;
    uint32_t cache = Swap32(*(uint32_t*)(buffer + byteOffset));
    // cache = cache << byteOffset1 >> byteOffset1;
    cache = cache << byteOffset1; // 0n 1 Xn ...
    int lg2 = log2(cache);
    int n = 32 - lg2 - 1;
    bitCounter += 2 * n + 1;
    if (n == 0)
        return 0;
    else
    {
        uint32_t ret = cache << (n + 1) >> (32 - n);
        ret += pow(2, n) - 1;
        return ret;
    }
}

uint32_t read_se_golomb(byte* buffer, int& bitCounter)
{
    auto ue = read_ue_golomb(buffer, bitCounter);
    if (ue & 1)
        return ue >> 1;
    else
        return -int32_t(ue >> 1);
}

#define MAXIMUMPARSETRBSPSIZE 1500
#define MAXIMUMPARSETNALUSIZE 1500

#define MAXSPS 32
#define MAXPPS 256

#define MAXIMUMVALUEOFcpb_cnt 32
#define bool int
typedef struct
{
    unsigned int cpb_cnt_minus1;                               // ue(v)
    unsigned int bit_rate_scale;                               // u(4)
    unsigned int cpb_size_scale;                               // u(4)
    unsigned int bit_rate_value_minus1[MAXIMUMVALUEOFcpb_cnt]; // ue(v)
    unsigned int cpb_size_value_minus1[MAXIMUMVALUEOFcpb_cnt]; // ue(v)
    unsigned int cbr_flag[MAXIMUMVALUEOFcpb_cnt];              // u(1)
    unsigned int initial_cpb_removal_delay_length_minus1;      // u(5)
    unsigned int cpb_removal_delay_length_minus1;              // u(5)
    unsigned int dpb_output_delay_length_minus1;               // u(5)
    unsigned int time_offset_length;                           // u(5)
} hrd_parameters_t;
typedef struct
{
    bool aspect_ratio_info_present_flag;              // u(1)
    unsigned int aspect_ratio_idc;                    // u(8)
    unsigned short sar_width;                         // u(16)
    unsigned short sar_height;                        // u(16)
    bool overscan_info_present_flag;                  // u(1)
    bool overscan_appropriate_flag;                   // u(1)
    bool video_signal_type_present_flag;              // u(1)
    unsigned int video_format;                        // u(3)
    bool video_full_range_flag;                       // u(1)
    bool colour_description_present_flag;             // u(1)
    unsigned int colour_primaries;                    // u(8)
    unsigned int transfer_characteristics;            // u(8)
    unsigned int matrix_coefficients;                 // u(8)
    bool chroma_location_info_present_flag;           // u(1)
    unsigned int chroma_sample_loc_type_top_field;    // ue(v)
    unsigned int chroma_sample_loc_type_bottom_field; // ue(v)
    bool timing_info_present_flag;                    // u(1)
    unsigned int num_units_in_tick;                   // u(32)
    unsigned int time_scale;                          // u(32)
    bool fixed_frame_rate_flag;                       // u(1)
    bool nal_hrd_parameters_present_flag;             // u(1)
    hrd_parameters_t nal_hrd_parameters;              // hrd_paramters_t
    bool vcl_hrd_parameters_present_flag;             // u(1)
    hrd_parameters_t vcl_hrd_parameters;              // hrd_paramters_t
    // if ((nal_hrd_parameters_present_flag || (vcl_hrd_parameters_present_flag))
    bool low_delay_hrd_flag;                      // u(1)
    bool pic_struct_present_flag;                 // u(1)
    bool bitstream_restriction_flag;              // u(1)
    bool motion_vectors_over_pic_boundaries_flag; // u(1)
    unsigned int max_bytes_per_pic_denom;         // ue(v)
    unsigned int max_bits_per_mb_denom;           // ue(v)
    unsigned int log2_max_mv_length_vertical;     // ue(v)
    unsigned int log2_max_mv_length_horizontal;   // ue(v)
    unsigned int max_num_reorder_frames;          // ue(v)
    unsigned int max_dec_frame_buffering;         // ue(v)
} vui_seq_parameters_t;
typedef struct
{
    bool Valid; // indicates the parameter set is valid

    unsigned int profile_idc;          // u(8)
    bool constrained_set0_flag;        // u(1)
    bool constrained_set1_flag;        // u(1)
    bool constrained_set2_flag;        // u(1)
    bool constrained_set3_flag;        // u(1)
    bool constrained_set4_flag;        // u(1)
    bool constrained_set5_flag;        // u(2)
    unsigned int level_idc;            // u(8)
    unsigned int seq_parameter_set_id; // ue(v)
    unsigned int chroma_format_idc;    // ue(v)

    bool seq_scaling_matrix_present_flag;  // u(1)
    int seq_scaling_list_present_flag[12]; // u(1)
    int ScalingList4x4[6][16];             // se(v)
    int ScalingList8x8[6][64];             // se(v)
    bool UseDefaultScalingMatrix4x4Flag[6];
    bool UseDefaultScalingMatrix8x8Flag[6];

    unsigned int bit_depth_luma_minus8;     // ue(v)
    unsigned int bit_depth_chroma_minus8;   // ue(v)
    unsigned int log2_max_frame_num_minus4; // ue(v)
    unsigned int pic_order_cnt_type;
    // if( pic_order_cnt_type == 0 )
    unsigned int log2_max_pic_order_cnt_lsb_minus4; // ue(v)
    // else if( pic_order_cnt_type == 1 )
    bool delta_pic_order_always_zero_flag;              // u(1)
    int offset_for_non_ref_pic;                         // se(v)
    int offset_for_top_to_bottom_field;                 // se(v)
    unsigned int num_ref_frames_in_pic_order_cnt_cycle; // ue(v)
    // for( i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
#define MAXnum_ref_frames_in_pic_order_cnt_cycle 256
    int offset_for_ref_frame[MAXnum_ref_frames_in_pic_order_cnt_cycle]; // se(v)
    unsigned int num_ref_frames;                                        // ue(v)
    bool gaps_in_frame_num_value_allowed_flag;                          // u(1)
    unsigned int pic_width_in_mbs_minus1;                               // ue(v)
    unsigned int pic_height_in_map_units_minus1;                        // ue(v)
    bool frame_mbs_only_flag;                                           // u(1)
    // if( !frame_mbs_only_flag )
    bool mb_adaptive_frame_field_flag;       // u(1)
    bool direct_8x8_inference_flag;          // u(1)
    bool frame_cropping_flag;                // u(1)
    unsigned int frame_crop_left_offset;     // ue(v)
    unsigned int frame_crop_right_offset;    // ue(v)
    unsigned int frame_crop_top_offset;      // ue(v)
    unsigned int frame_crop_bottom_offset;   // ue(v)
    bool vui_parameters_present_flag;        // u(1)
    vui_seq_parameters_t vui_seq_parameters; // vui_seq_parameters_t
    unsigned separate_colour_plane_flag;     // u(1)
#if (MVC_EXTENSION_ENABLE)
    int max_dec_frame_buffering;
#endif
    int lossless_qpprime_flag;
} seq_parameter_set_rbsp_t;

bool ValIn(int val, std::vector<int> cands)
{
    for (auto can : cands)
        if (can == val)
            return true;
    return false;
}

#define GetBit(n) get_bits(buf, bitCounter, n)
#define read_ue() read_ue_golomb(buf, bitCounter)
#define read_se() read_se_golomb(buf, bitCounter)

void InterpretHRD(DataView view, hrd_parameters_t& hrd, int& bitCounter)
{
    auto buf = view.buf;

    hrd.cpb_cnt_minus1 = read_ue();
    hrd.bit_rate_scale = GetBit(4);
    hrd.cpb_size_scale = GetBit(4);
    for (int schedSe1Idx = 0; schedSe1Idx < hrd.cpb_cnt_minus1; ++schedSe1Idx)
    {
        hrd.bit_rate_value_minus1[schedSe1Idx] = read_ue();
        hrd.cpb_size_value_minus1[schedSe1Idx] = read_ue();
        hrd.cbr_flag[schedSe1Idx] = GetBit(1);
    }
    hrd.initial_cpb_removal_delay_length_minus1 = GetBit(5);
    hrd.cpb_removal_delay_length_minus1 = GetBit(5);
    hrd.dpb_output_delay_length_minus1 = GetBit(5);
    hrd.time_offset_length = GetBit(5);
}

/* $spec:[E.1.1] */
void InterpretVUI(DataView view, vui_seq_parameters_t& vui, int& bitCounter)
{
    vui.matrix_coefficients = 2;

    auto buf = view.buf;
    if (vui.aspect_ratio_info_present_flag = GetBit(1))
    {
        vui.aspect_ratio_idc = GetBit(8);
        /* $spec:[Table E-1] Extended_SAR=255 */
        if (vui.aspect_ratio_idc == 255)
        {
            vui.sar_width = GetBit(16);
            vui.sar_height = GetBit(16);
        }
    }
    if (vui.overscan_appropriate_flag = GetBit(1))
        vui.overscan_appropriate_flag = GetBit(1);
    if (vui.video_signal_type_present_flag = GetBit(1))
    {
        vui.video_format = GetBit(3);
        vui.video_full_range_flag = GetBit(1);
        if (vui.colour_description_present_flag = GetBit(1))
        {
            vui.colour_primaries = GetBit(8);
            vui.transfer_characteristics = GetBit(8);
            vui.matrix_coefficients = GetBit(8);
        }
    }
    if (vui.chroma_location_info_present_flag = GetBit(1))
    {
        vui.chroma_sample_loc_type_top_field = read_ue();
        vui.chroma_sample_loc_type_bottom_field = read_ue();
    }
    if (vui.timing_info_present_flag = GetBit(1))
    {
        vui.num_units_in_tick = GetBit(32);
        vui.time_scale = GetBit(32);
        vui.fixed_frame_rate_flag = GetBit(1);
    }
    if (vui.nal_hrd_parameters_present_flag = GetBit(1))
        InterpretHRD(view, vui.nal_hrd_parameters, bitCounter);
    if (vui.vcl_hrd_parameters_present_flag = GetBit(1))
        InterpretHRD(view, vui.vcl_hrd_parameters, bitCounter);

    if (vui.nal_hrd_parameters_present_flag || vui.vcl_hrd_parameters_present_flag)
        vui.low_delay_hrd_flag = GetBit(1);
    vui.pic_struct_present_flag = GetBit(1);
    if (vui.bitstream_restriction_flag = GetBit(1))
    {
        vui.motion_vectors_over_pic_boundaries_flag = GetBit(1);
        vui.max_bytes_per_pic_denom = read_ue();
        vui.max_bits_per_mb_denom = read_ue();
        vui.log2_max_mv_length_horizontal = read_ue();
        vui.log2_max_mv_length_vertical = read_ue();
        vui.max_num_reorder_frames = read_ue();
        vui.max_dec_frame_buffering = read_ue();
    }
}
void InterpretSPS(DataView view, seq_parameter_set_rbsp_t& sps, int& bitCounter)
{
    byte* buf = view.buf;
    // while (*buf++ != 0x01)
    //     ;
    sps.profile_idc = GetBit(8);
    sps.constrained_set0_flag = GetBit(1);
    sps.constrained_set1_flag = GetBit(1);
    sps.constrained_set2_flag = GetBit(1);
    sps.constrained_set3_flag = GetBit(1);
    sps.constrained_set4_flag = GetBit(1);
    sps.constrained_set5_flag = GetBit(1);
    int reserved_2bits = GetBit(2);
    sps.level_idc = GetBit(8);
    sps.seq_parameter_set_id = read_ue();
    if (ValIn(sps.profile_idc, { 100, 110, 122, 244, 44, 83, 86, 118, 128, 138, 139, 134, 135 }))
    {
        sps.chroma_format_idc = read_ue();
        if (sps.chroma_format_idc == 3)
            sps.separate_colour_plane_flag = GetBit(1);
        sps.bit_depth_luma_minus8 = read_ue();
        sps.bit_depth_chroma_minus8 = read_ue();
        sps.lossless_qpprime_flag = GetBit(1);
        sps.seq_scaling_matrix_present_flag = GetBit(1);
        if (sps.seq_scaling_matrix_present_flag)
        {
            for (int i = 0; i < (sps.chroma_format_idc != 3 ? 8 : 12); i++)
            {
                sps.seq_scaling_list_present_flag[i] = GetBit(1);
                if (sps.seq_scaling_list_present_flag[i])
                {
                    if (i < 6)
                        ; //
                    else
                        ; //
                }
            }
        }
    }
    sps.log2_max_frame_num_minus4 = read_ue();
    sps.pic_order_cnt_type = read_ue();
    if (sps.pic_order_cnt_type == 0)
        sps.log2_max_pic_order_cnt_lsb_minus4 = read_ue();
    else if (sps.pic_order_cnt_type == 1)
    {
        sps.delta_pic_order_always_zero_flag = GetBit(1);
        sps.offset_for_non_ref_pic = read_se();
        sps.offset_for_top_to_bottom_field = read_se();
        sps.num_ref_frames_in_pic_order_cnt_cycle = read_ue();
        for (int i = 0; i < sps.num_ref_frames_in_pic_order_cnt_cycle; i++)
            sps.offset_for_ref_frame[i] = read_se();
    }
    sps.num_ref_frames = read_ue();
    sps.gaps_in_frame_num_value_allowed_flag = GetBit(1);
    sps.pic_width_in_mbs_minus1 = read_ue();
    sps.pic_height_in_map_units_minus1 = read_ue();
    sps.frame_mbs_only_flag = GetBit(1);
    if (!sps.frame_mbs_only_flag)
        sps.mb_adaptive_frame_field_flag = GetBit(1);
    sps.direct_8x8_inference_flag = GetBit(1);
    sps.frame_cropping_flag = GetBit(1);
    if (sps.frame_cropping_flag)
    {
        sps.frame_crop_left_offset = read_ue();
        sps.frame_crop_right_offset = read_ue();
        sps.frame_crop_top_offset = read_ue();
        sps.frame_crop_bottom_offset = read_ue();
    }
    if (sps.vui_parameters_present_flag = GetBit(1))
        InterpretVUI(view, sps.vui_seq_parameters, bitCounter);
}

int main()
{
    auto start = std::chrono::system_clock::now();
    auto atExit = makeExit([&] {
        auto now = std::chrono::system_clock::now();
        printf("cost %lldms\n", std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count()); });

    for (int i = 0; i < 100; ++i)
    {
        AnnexReader reader;
        reader.Init(CMAKE_SOURCE_DIR R"(../h264_win/res/s.h264)");
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
            NaluType type = NaluType(bufView.buf[x] & 0x1f);
            nalu_count++;
            frame_count += type <= 5;
            LOG("meet nalu:%4d f:%d type=%2d(%3x)   size=%5d    pos=%d\n",
                nalu_count,
                frame_count,
                type,
                uint32_t(bufView.buf[x]),
                bufView.sz,
                bufView.file_pos);

            auto refNal = makeRefNal(bufView);
            DataView rbsp_view(refNal);
            seq_parameter_set_rbsp_t sps;
            memset(&sps, 0, sizeof(sps));
            int bitCounter = 0;

            switch (type)
            {
            case NALU_TYPE_SPS:
                InterpretSPS(rbsp_view, sps, bitCounter);
                LOG("meet SPS");
                break;
            default:
                break;
            }
        }
        reader.UnInit();
    }
}
