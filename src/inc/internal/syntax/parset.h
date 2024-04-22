#pragma once
#include <c264/c264.h>
#include <memory>
#include <syntax/common.h>

#define MAXIMUMPARSETRBSPSIZE 1500
#define MAXIMUMPARSETNALUSIZE 1500

#define MAXSPS 32
#define MAXPPS 256

#define MAXIMUMVALUEOFcpb_cnt 32
using Bool = int;
// clang-format off

// note: field Scan list is not seen yet, log it when it does

// $spec[8.5.6] Inverse scanning process for 4x4 transform coefficients and scaling lists
// $spec[Figure 8-8 ] 4x4 block scans. (a) Zig-zag scan. 
static const uint8_t g_kuiZigzagScan[16] = { 
  0,  1,  4,  8,
  5,  2,  3,  6,
  9, 12, 13, 10,
  7, 11, 14, 15,
};

// $spec[8.5.7] Inverse scanning process for 8x8 transform coefficients and scaling lists
// $spec[Figure 8-9 ] 8x8 block scans. (a) 8x8 zig-zag scan.
static const uint8_t g_kuiZigzagScan8x8[64] = { //8x8 block residual zig-zag scan order
  0,  1,  8,  16, 9,  2,  3,  10,
  17, 24, 32, 25, 18, 11, 4,  5,
  12, 19, 26, 33, 40, 48, 41, 34,
  27, 20, 13, 6,  7,  14, 21, 28,
  35, 42, 49, 56, 57, 50, 43, 36,
  29, 22, 15, 23, 30, 37, 44, 51,
  58, 59, 52, 45, 38, 31, 39, 46,
  53, 60, 61, 54, 47, 55, 62, 63,
};
enum class ChormaFormatIDC{
    Monochronome,
    XYZ420,
    XYZ422,
    XYZ444,
};
struct ChromaFormatInfo
{
    ChormaFormatIDC chroma_format_idc;
    int SubWidthC;
    int SubHeightC;
};
extern const ChromaFormatInfo g_chromaFormatInfos[4];
// clang-format on

struct hrd_parameters_t
{
    uint32_t cpb_cnt_minus1;                               // ue(v)
    uint32_t bit_rate_scale;                               // u(4)
    uint32_t cpb_size_scale;                               // u(4)
    uint32_t bit_rate_value_minus1[MAXIMUMVALUEOFcpb_cnt]; // ue(v)
    uint32_t cpb_size_value_minus1[MAXIMUMVALUEOFcpb_cnt]; // ue(v)
    uint32_t cbr_flag[MAXIMUMVALUEOFcpb_cnt];              // u(1)
    uint32_t initial_cpb_removal_delay_length_minus1;      // u(5)
    uint32_t cpb_removal_delay_length_minus1;              // u(5)
    uint32_t dpb_output_delay_length_minus1;               // u(5)
    uint32_t time_offset_length;                           // u(5)
};

struct vui_seq_parameters_t
{
    Bool             aspect_ratio_info_present_flag;      // u(1)
    uint32_t         aspect_ratio_idc;                    // u(8)
    uint16_t         sar_width;                           // u(16)
    uint16_t         sar_height;                          // u(16)
    Bool             overscan_info_present_flag;          // u(1)
    Bool             overscan_appropriate_flag;           // u(1)
    Bool             video_signal_type_present_flag;      // u(1)
    uint32_t         video_format;                        // u(3)
    Bool             video_full_range_flag;               // u(1)
    Bool             colour_description_present_flag;     // u(1)
    uint32_t         colour_primaries;                    // u(8)
    uint32_t         transfer_characteristics;            // u(8)
    uint32_t         matrix_coefficients;                 // u(8)
    Bool             chroma_location_info_present_flag;   // u(1)
    uint32_t         chroma_sample_loc_type_top_field;    // ue(v)
    uint32_t         chroma_sample_loc_type_bottom_field; // ue(v)
    Bool             timing_info_present_flag;            // u(1)
    uint32_t         num_units_in_tick;                   // u(32)
    uint32_t         time_scale;                          // u(32)
    Bool             fixed_frame_rate_flag;               // u(1)
    Bool             nal_hrd_parameters_present_flag;     // u(1)
    hrd_parameters_t nal_hrd_parameters;                  // hrd_paramters_t
    Bool             vcl_hrd_parameters_present_flag;     // u(1)
    hrd_parameters_t vcl_hrd_parameters;                  // hrd_paramters_t
    // if ((nal_hrd_parameters_present_flag || (vcl_hrd_parameters_present_flag))
    Bool     low_delay_hrd_flag;                      // u(1)
    Bool     pic_struct_present_flag;                 // u(1)
    Bool     bitstream_restriction_flag;              // u(1)
    Bool     motion_vectors_over_pic_boundaries_flag; // u(1)
    uint32_t max_bytes_per_pic_denom;                 // ue(v)
    uint32_t max_bits_per_mb_denom;                   // ue(v)
    uint32_t log2_max_mv_length_vertical;             // ue(v)
    uint32_t log2_max_mv_length_horizontal;           // ue(v)
    uint32_t max_num_reorder_frames;                  // ue(v)
    uint32_t max_dec_frame_buffering;                 // ue(v)
};

struct seq_parameter_set_rbsp_t
{
    uint32_t profile_idc;           // u(8)
    Bool     constrained_set0_flag; // u(1)
    Bool     constrained_set1_flag; // u(1)
    Bool     constrained_set2_flag; // u(1)
    Bool     constrained_set3_flag; // u(1)
    Bool     constrained_set4_flag; // u(1)
    Bool     constrained_set5_flag; // u(2)
    uint32_t level_idc;             // u(8)
    uint32_t seq_parameter_set_id;  // ue(v)
    uint8_t  chroma_format_idc;     // ue(v)

    Bool seq_scaling_matrix_present_flag;   // u(1)
    int  seq_scaling_list_present_flag[12]; // u(1)
    int  ScalingList4x4[6][16];             // se(v)
    int  ScalingList8x8[6][64];             // se(v)
    Bool UseDefaultScalingMatrix4x4Flag[6];
    Bool UseDefaultScalingMatrix8x8Flag[6];

    uint32_t bit_depth_luma_minus8;     // ue(v)
    uint32_t bit_depth_chroma_minus8;   // ue(v)
    uint32_t log2_max_frame_num_minus4; // ue(v)
    uint32_t pic_order_cnt_type;
    // if( pic_order_cnt_type == 0 )
    uint32_t log2_max_pic_order_cnt_lsb_minus4; // ue(v)
    // else if( pic_order_cnt_type == 1 )
    Bool     delta_pic_order_always_zero_flag;      // u(1)
    int      offset_for_non_ref_pic;                // se(v)
    int      offset_for_top_to_bottom_field;        // se(v)
    uint32_t num_ref_frames_in_pic_order_cnt_cycle; // ue(v)
    // for( i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
#define MAXnum_ref_frames_in_pic_order_cnt_cycle 256
    int      offset_for_ref_frame[MAXnum_ref_frames_in_pic_order_cnt_cycle]; // se(v)
    uint32_t num_ref_frames;                                                 // ue(v)
    Bool     gaps_in_frame_num_value_allowed_flag;                           // u(1)
    uint32_t pic_width_in_mbs_minus1;                                        // ue(v)
    uint32_t pic_height_in_map_units_minus1;                                 // ue(v)
    Bool     frame_mbs_only_flag;                                            // u(1)
    // if( !frame_mbs_only_flag )
    Bool                 mb_adaptive_frame_field_flag; // u(1)
    Bool                 direct_8x8_inference_flag;    // u(1)
    Bool                 frame_cropping_flag;          // u(1)
    uint32_t             frame_crop_left_offset;       // ue(v)
    uint32_t             frame_crop_right_offset;      // ue(v)
    uint32_t             frame_crop_top_offset;        // ue(v)
    uint32_t             frame_crop_bottom_offset;     // ue(v)
    Bool                 vui_parameters_present_flag;  // u(1)
    vui_seq_parameters_t vui_seq_parameters;           // vui_seq_parameters_t
    unsigned             separate_colour_plane_flag;   // u(1)
    int                  max_dec_frame_buffering;
    int                  lossless_qpprime_flag;
    // this may be renamed from lossless_qpprime_flag
#define qpprime_y_zero_transform_bypass_flag lossless_qpprime_flag
};

#define MAXnum_slice_groups_minus1 8
struct pic_parameter_set_rbsp_t
{
    uint32_t pic_parameter_set_id;                         // ue(v)
    uint32_t seq_parameter_set_id;                         // ue(v)
    Bool     entropy_coding_mode_flag;                     // u(1)
    Bool     bottom_field_pic_order_in_frame_present_flag; // u(1)
    uint32_t num_slice_groups_minus1;                      // ue(v)

    uint32_t slice_group_map_type;                          // ue(v)
    uint32_t run_length_minus1[MAXnum_slice_groups_minus1]; // ue(v)
    uint32_t top_left[MAXnum_slice_groups_minus1];          // ue(v)
    uint32_t bottom_right[MAXnum_slice_groups_minus1];      // ue(v)
    Bool     slice_group_change_direction_flag;             // u(1)
    uint32_t slice_group_change_rate_minus1;                // ue(v)
    uint32_t pic_size_in_map_units_minus1;                  // ue(v)
    byte*    slice_group_id;                                // u(v)

    int      num_ref_idx_l0_default_active_minus1;   // ue(v)
    int      num_ref_idx_l1_default_active_minus1;   // ue(v)
    Bool     weighted_pred_flag;                     // u(1)
    uint32_t weighted_bipred_idc;                    // u(2)
    int      pic_init_qp_minus26;                    // se(v)
    int      pic_init_qs_minus26;                    // se(v)
    int      chroma_qp_index_offset;                 // se(v)
    Bool     deblocking_filter_control_present_flag; // u(1)
    Bool     constrained_intra_pred_flag;            // u(1)
    Bool     redundant_pic_cnt_present_flag;         // u(1)

    Bool transform_8x8_mode_flag;           // u(1)
    Bool pic_scaling_matrix_present_flag;   // u(1)
    int  pic_scaling_list_present_flag[12]; // u(1)
    // int  ScalingList4x4[6][16];             // se(v)
    // int  ScalingList8x8[6][64];             // se(v)
    // Bool UseDefaultScalingMatrix4x4Flag[6];
    // Bool UseDefaultScalingMatrix8x8Flag[6];

    int second_chroma_qp_index_offset; // se(v)

    int cb_qp_index_offset; // se(v)
    int cr_qp_index_offset; // se(v)
};

struct Seq_parameter_set_rbsp_t : public seq_parameter_set_rbsp_t
{
    struct Deduced
    {
        int MbWidthC;
        int MbHeightC;
        int BitDepth_Y;
        int QpBdOffset_Y;
        int BitDepth_C;
        int QpBdOffset_C;
    } dd;
};

std::unique_ptr<Seq_parameter_set_rbsp_t> ParseSPS(RBSPCursor& cursor);
std::unique_ptr<pic_parameter_set_rbsp_t> ParsePPS(RBSPCursor& cursor, const struct Parser* parser);