#pragma once
#include <c264/c264.h>
#include <syntax/common.h>

#define MAXIMUMPARSETRBSPSIZE 1500
#define MAXIMUMPARSETNALUSIZE 1500

#define MAXSPS 32
#define MAXPPS 256

#define MAXIMUMVALUEOFcpb_cnt 32
using Bool = int;

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
    Bool             aspect_ratio_info_present_flag;      // u(1)
    unsigned int     aspect_ratio_idc;                    // u(8)
    unsigned short   sar_width;                           // u(16)
    unsigned short   sar_height;                          // u(16)
    Bool             overscan_info_present_flag;          // u(1)
    Bool             overscan_appropriate_flag;           // u(1)
    Bool             video_signal_type_present_flag;      // u(1)
    unsigned int     video_format;                        // u(3)
    Bool             video_full_range_flag;               // u(1)
    Bool             colour_description_present_flag;     // u(1)
    unsigned int     colour_primaries;                    // u(8)
    unsigned int     transfer_characteristics;            // u(8)
    unsigned int     matrix_coefficients;                 // u(8)
    Bool             chroma_location_info_present_flag;   // u(1)
    unsigned int     chroma_sample_loc_type_top_field;    // ue(v)
    unsigned int     chroma_sample_loc_type_bottom_field; // ue(v)
    Bool             timing_info_present_flag;            // u(1)
    unsigned int     num_units_in_tick;                   // u(32)
    unsigned int     time_scale;                          // u(32)
    Bool             fixed_frame_rate_flag;               // u(1)
    Bool             nal_hrd_parameters_present_flag;     // u(1)
    hrd_parameters_t nal_hrd_parameters;                  // hrd_paramters_t
    Bool             vcl_hrd_parameters_present_flag;     // u(1)
    hrd_parameters_t vcl_hrd_parameters;                  // hrd_paramters_t
    // if ((nal_hrd_parameters_present_flag || (vcl_hrd_parameters_present_flag))
    Bool         low_delay_hrd_flag;                      // u(1)
    Bool         pic_struct_present_flag;                 // u(1)
    Bool         bitstream_restriction_flag;              // u(1)
    Bool         motion_vectors_over_pic_boundaries_flag; // u(1)
    unsigned int max_bytes_per_pic_denom;                 // ue(v)
    unsigned int max_bits_per_mb_denom;                   // ue(v)
    unsigned int log2_max_mv_length_vertical;             // ue(v)
    unsigned int log2_max_mv_length_horizontal;           // ue(v)
    unsigned int max_num_reorder_frames;                  // ue(v)
    unsigned int max_dec_frame_buffering;                 // ue(v)
} vui_seq_parameters_t;
typedef struct
{
    Bool Valid; // indicates the parameter set is valid

    unsigned int profile_idc;           // u(8)
    Bool         constrained_set0_flag; // u(1)
    Bool         constrained_set1_flag; // u(1)
    Bool         constrained_set2_flag; // u(1)
    Bool         constrained_set3_flag; // u(1)
    Bool         constrained_set4_flag; // u(1)
    Bool         constrained_set5_flag; // u(2)
    unsigned int level_idc;             // u(8)
    unsigned int seq_parameter_set_id;  // ue(v)
    unsigned int chroma_format_idc;     // ue(v)

    Bool seq_scaling_matrix_present_flag;   // u(1)
    int  seq_scaling_list_present_flag[12]; // u(1)
    int  ScalingList4x4[6][16];             // se(v)
    int  ScalingList8x8[6][64];             // se(v)
    Bool UseDefaultScalingMatrix4x4Flag[6];
    Bool UseDefaultScalingMatrix8x8Flag[6];

    unsigned int bit_depth_luma_minus8;     // ue(v)
    unsigned int bit_depth_chroma_minus8;   // ue(v)
    unsigned int log2_max_frame_num_minus4; // ue(v)
    unsigned int pic_order_cnt_type;
    // if( pic_order_cnt_type == 0 )
    unsigned int log2_max_pic_order_cnt_lsb_minus4; // ue(v)
    // else if( pic_order_cnt_type == 1 )
    Bool         delta_pic_order_always_zero_flag;      // u(1)
    int          offset_for_non_ref_pic;                // se(v)
    int          offset_for_top_to_bottom_field;        // se(v)
    unsigned int num_ref_frames_in_pic_order_cnt_cycle; // ue(v)
    // for( i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
#define MAXnum_ref_frames_in_pic_order_cnt_cycle 256
    int          offset_for_ref_frame[MAXnum_ref_frames_in_pic_order_cnt_cycle]; // se(v)
    unsigned int num_ref_frames;                                                 // ue(v)
    Bool         gaps_in_frame_num_value_allowed_flag;                           // u(1)
    unsigned int pic_width_in_mbs_minus1;                                        // ue(v)
    unsigned int pic_height_in_map_units_minus1;                                 // ue(v)
    Bool         frame_mbs_only_flag;                                            // u(1)
    // if( !frame_mbs_only_flag )
    Bool                 mb_adaptive_frame_field_flag; // u(1)
    Bool                 direct_8x8_inference_flag;    // u(1)
    Bool                 frame_cropping_flag;          // u(1)
    unsigned int         frame_crop_left_offset;       // ue(v)
    unsigned int         frame_crop_right_offset;      // ue(v)
    unsigned int         frame_crop_top_offset;        // ue(v)
    unsigned int         frame_crop_bottom_offset;     // ue(v)
    Bool                 vui_parameters_present_flag;  // u(1)
    vui_seq_parameters_t vui_seq_parameters;           // vui_seq_parameters_t
    unsigned             separate_colour_plane_flag;   // u(1)
    int                  max_dec_frame_buffering;
    int                  lossless_qpprime_flag;
} seq_parameter_set_rbsp_t;

seq_parameter_set_rbsp_t* ParseSPS(RBSPCursor& cursor);