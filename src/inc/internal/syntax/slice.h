#pragma once
#include <c264/c264.h>
#include <syntax/common.h>
#include <syntax/cabac.h>
#include <syntax/macroblock.h>

#define MAX_REF_PIC_COUNT 16 // MAX Short + Long reference pictures
#define MAX_MMCO_COUNT 66

enum SliceType
{
    SLICE_P = 0,
    SLICE_B = 1,
    SLICE_I = 2,
    SLICE_SP = 3,
    SLICE_SI = 4,
    SLICE_TYPE_CNT = 5
};

/* List Index */
enum EListIndex
{
    LIST_0 = 0,
    LIST_1 = 1,
    LIST_CNT = 2
};

/* Colour Index */
enum ColourIndex
{
    COL_LUMA,
    COL_CHROMA0,
    COL_CHROMA1,
    COLOR_INDEX_CNT
};

enum ColorPlane
{
    // YUV
    PLANE_Y = 0, // PLANE_Y
    PLANE_U = 1, // PLANE_Cb
    PLANE_V = 2, // PLANE_Cr
};

struct slice_t
{
    int       first_mb_in_slice;          // ue(v)
    SliceType slice_type;                 // ue(v)
    int       pic_parameter_set_id;       // ue(v)
    int       colour_plane_id;            // u(2)
    int       frame_num;                  // u(v)
    int       field_pic_flag;             // u(1)
    int       bottom_field_flag;          // u(1)
    int       idr_pic_id;                 // ue(v)
    int       pic_order_cnt_lsb;          // u(v)
    int       delta_pic_order_cnt_bottom; // se(v)

    int delta_pic_order_cnt[2];              // se(v)
    int redundant_pic_cnt;                   // ue(v)
    int direct_spatial_mv_pred_flag;         // u(1)
    int num_ref_idx_active_override_flag;    // u(1)
    int num_ref_idx_active_minus1[LIST_CNT]; // ue(v) num_ref_idx_l0_active_minus1 + num_ref_idx_l1_active_minus1

    // $spec [7.3.3.1] Reference picture list modification syntax
    int ref_pic_list_modification_flag[LIST_CNT];                  // u(1)
    int modification_of_pic_nums_idc[LIST_CNT][MAX_REF_PIC_COUNT]; // ue(v) num_ref_idx_l[01]_active_minus1
    int abs_diff_pic_num_minus1[LIST_CNT][MAX_REF_PIC_COUNT];      // ue(v)
    int long_term_pic_num[LIST_CNT][MAX_REF_PIC_COUNT];            // ue(v)

    // $spec [7.3.3.2] Prediction weight table syntax
    uint16_t luma_log2_weight_denom;
    uint16_t chroma_log2_weight_denom;
    int      luma_weight_flag[LIST_CNT];                              // luma_weight_flag_l0 + luma_weight_flag_l1
    int      chroma_weight_flag[LIST_CNT];                            // chroma_weight_flag_l0 + chroma_weight_flag_l1
    int      wp_weight[LIST_CNT][MAX_REF_PIC_COUNT][COLOR_INDEX_CNT]; // luma_weight chroma_weight
    int      wp_offset[LIST_CNT][MAX_REF_PIC_COUNT][COLOR_INDEX_CNT]; // luma_offset chroma_offset

    // $spec [7.3.3.3] Decoded reference pircture making syntax
    int no_output_of_prior_pics_flag;
    int long_term_reference_flag;
    int adaptive_ref_pic_buffering_flag;
    struct decRefPicMarking_t
    {
        int memory_management_control_operation;
        int difference_of_pic_nums_minus1;
        int long_term_pic_num;
        int long_term_frame_idx;
        int max_long_term_frame_idx_plus1;
    } decRefPicMarking[MAX_MMCO_COUNT];

    int cabac_init_idc;     // ue(v)
    int slice_qp_delta;     // se(v)
    int sp_for_switch_flag; // u(1)
    int slice_qs_delta;     // se(v)

    int disable_deblocking_filter_idc; // ue(v)
    int slice_alpha_c0_offset_div2;    // se(v)
    int slice_beta_offset_div2;        // se(v)

    int slice_group_change_cycle; // u(v)
};

struct Slice_t : public slice_t
{
    ~Slice_t();
    const struct nalu_t*                   nalu;
    const struct pic_parameter_set_rbsp_t* pps;
    const struct Seq_parameter_set_rbsp_t* sps;

    struct Deduced
    {
        Dp_Mode dp_mode;
        bool    MbaffFrameFlag;
        int     SliceQP_Y;
        int     QS_Y;
        int     FilterOffsetA;
        int     FilterOffsetB;
        int     PicWidthInMbs; // note fielded frame got two Pic
        int     PicHeightInMapUnits;
        int     FrameHeightInMbs;
        int     FrameSizeInMbs;

        std::vector<BlockPos> PicPos; // see jm: init_global_buffers cps->PicPos
    } dd;

    RBSPCursor rbspCursor;
    struct BACContext
    {
        BACDecoder      decoder;
        ContextVariable bac_contexts[CABAC_CONTEXT_CNT];
    } cabac;

    struct VLCContext{
        GetBitContext gbc;
    } cavlc;

    std::vector<Macroblock_t> mbs_buf;
    Macroblock_t*             mbs;
    int                       mb_line_stride;

    int curMBIdx;

    /* 
    *  DBC
    *  AX
    */
    int mbAddrA;
    int mbAddrB;
    int mbAddrC;
    int mbAddrD;
    int mbTypeA;
    int mbTypeD;
    int mbTypeC;
    int mbTypeB;
#define mbTypeLeft mbTypeA
#define mbTypeTop mbTypeB
#define mbTypeTopRight mbTypeC
#define mbTypeTopLeft mbTypeD
#define mbAddrLeft mbAddrA
#define mbAddrTop mbAddrB
#define mbAddrTopRight mbAddrC
#define mbAddrTopLeft mbAddrD

    int (*fnDecodeBlock)(Macroblock_t* curMB);
};
std::unique_ptr<Slice_t> ParseSliceHeader(RBSPCursor& cursor, const struct Parser* parser, const nalu_t* nalu);