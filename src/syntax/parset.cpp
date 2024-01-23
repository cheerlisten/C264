#include <syntax/parset.h>
#include <log.h>

static void InterpretHRD(hrd_parameters_t& hrd, RBSPCursor& cursor)
{
    StartBits(cursor.buf, cursor.bit_pos);

    hrd.cpb_cnt_minus1 = read_ue();
    hrd.bit_rate_scale = GetBits(4);
    hrd.cpb_size_scale = GetBits(4);
    for (int schedSe1Idx = 0; schedSe1Idx < hrd.cpb_cnt_minus1; ++schedSe1Idx)
    {
        hrd.bit_rate_value_minus1[schedSe1Idx] = read_ue();
        hrd.cpb_size_value_minus1[schedSe1Idx] = read_ue();
        hrd.cbr_flag[schedSe1Idx] = GetBits(1);
    }
    hrd.initial_cpb_removal_delay_length_minus1 = GetBits(5);
    hrd.cpb_removal_delay_length_minus1 = GetBits(5);
    hrd.dpb_output_delay_length_minus1 = GetBits(5);
    hrd.time_offset_length = GetBits(5);
}

static void InterpretVUI(vui_seq_parameters_t& vui, RBSPCursor& cursor)
{
    vui.matrix_coefficients = 2;

    StartBits(cursor.buf, cursor.bit_pos);

    if (vui.aspect_ratio_info_present_flag = GetBits(1))
    {
        vui.aspect_ratio_idc = GetBits(8);
        /* $spec:[Table E-1] Extended_SAR=255 */
        if (vui.aspect_ratio_idc == 255)
        {
            vui.sar_width = GetBits(16);
            vui.sar_height = GetBits(16);
        }
    }
    if (vui.overscan_appropriate_flag = GetBits(1))
        vui.overscan_appropriate_flag = GetBits(1);
    if (vui.video_signal_type_present_flag = GetBits(1))
    {
        vui.video_format = GetBits(3);
        vui.video_full_range_flag = GetBits(1);
        if (vui.colour_description_present_flag = GetBits(1))
        {
            vui.colour_primaries = GetBits(8);
            vui.transfer_characteristics = GetBits(8);
            vui.matrix_coefficients = GetBits(8);
        }
    }
    if (vui.chroma_location_info_present_flag = GetBits(1))
    {
        vui.chroma_sample_loc_type_top_field = read_ue();
        vui.chroma_sample_loc_type_bottom_field = read_ue();
    }
    if (vui.timing_info_present_flag = GetBits(1))
    {
        vui.num_units_in_tick = GetBits(32);
        vui.time_scale = GetBits(32);
        vui.fixed_frame_rate_flag = GetBits(1);
    }
    if (vui.nal_hrd_parameters_present_flag = GetBits(1))
        InterpretHRD(vui.nal_hrd_parameters, cursor);
    if (vui.vcl_hrd_parameters_present_flag = GetBits(1))
        InterpretHRD(vui.vcl_hrd_parameters, cursor);

    if (vui.nal_hrd_parameters_present_flag || vui.vcl_hrd_parameters_present_flag)
        vui.low_delay_hrd_flag = GetBits(1);
    vui.pic_struct_present_flag = GetBits(1);
    if (vui.bitstream_restriction_flag = GetBits(1))
    {
        vui.motion_vectors_over_pic_boundaries_flag = GetBits(1);
        vui.max_bytes_per_pic_denom = read_ue();
        vui.max_bits_per_mb_denom = read_ue();
        vui.log2_max_mv_length_horizontal = read_ue();
        vui.log2_max_mv_length_vertical = read_ue();
        vui.max_num_reorder_frames = read_ue();
        vui.max_dec_frame_buffering = read_ue();
    }
}

seq_parameter_set_rbsp_t* ParseSPS(RBSPCursor& cursor)
{
    seq_parameter_set_rbsp_t* sps = new seq_parameter_set_rbsp_t;
    memset(sps, 0, sizeof(*sps));

    RBSPCursor __cursor = cursor;
    StartBits(__cursor.buf, __cursor.bit_pos);

    sps->profile_idc = GetBits(8);
    sps->constrained_set0_flag = GetBits(1);
    sps->constrained_set1_flag = GetBits(1);
    sps->constrained_set2_flag = GetBits(1);
    sps->constrained_set3_flag = GetBits(1);
    sps->constrained_set4_flag = GetBits(1);
    sps->constrained_set5_flag = GetBits(1);
    int reserved_2bits = GetBits(2);
    sps->level_idc = GetBits(8);
    sps->seq_parameter_set_id = read_ue();

    if (EqualToAnyOf(sps->profile_idc, 100, 110, 122, 244, 44, 83, 86, 118, 128, 138, 139, 134, 135))
    {
        sps->chroma_format_idc = read_ue();
        if (sps->chroma_format_idc == 3)
            sps->separate_colour_plane_flag = GetBits(1);
        sps->bit_depth_luma_minus8 = read_ue();
        sps->bit_depth_chroma_minus8 = read_ue();
        sps->lossless_qpprime_flag = GetBits(1);
        sps->seq_scaling_matrix_present_flag = GetBits(1);
        if (sps->seq_scaling_matrix_present_flag)
        {
            for (int i = 0; i < (sps->chroma_format_idc != 3 ? 8 : 12); i++)
            {
                sps->seq_scaling_list_present_flag[i] = GetBits(1);
                if (sps->seq_scaling_list_present_flag[i])
                {
                    if (i < 6)
                        ; //
                    else
                        ; //
                }
            }
        }
    }
    sps->log2_max_frame_num_minus4 = read_ue();
    sps->pic_order_cnt_type = read_ue();
    if (sps->pic_order_cnt_type == 0)
        sps->log2_max_pic_order_cnt_lsb_minus4 = read_ue();
    else if (sps->pic_order_cnt_type == 1)
    {
        sps->delta_pic_order_always_zero_flag = GetBits(1);
        sps->offset_for_non_ref_pic = read_se();
        sps->offset_for_top_to_bottom_field = read_se();
        sps->num_ref_frames_in_pic_order_cnt_cycle = read_ue();
        for (int i = 0; i < sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
            sps->offset_for_ref_frame[i] = read_se();
    }
    sps->num_ref_frames = read_ue();
    sps->gaps_in_frame_num_value_allowed_flag = GetBits(1);
    sps->pic_width_in_mbs_minus1 = read_ue();
    sps->pic_height_in_map_units_minus1 = read_ue();
    sps->frame_mbs_only_flag = GetBits(1);
    if (!sps->frame_mbs_only_flag)
        sps->mb_adaptive_frame_field_flag = GetBits(1);
    sps->direct_8x8_inference_flag = GetBits(1);
    sps->frame_cropping_flag = GetBits(1);
    if (sps->frame_cropping_flag)
    {
        sps->frame_crop_left_offset = read_ue();
        sps->frame_crop_right_offset = read_ue();
        sps->frame_crop_top_offset = read_ue();
        sps->frame_crop_bottom_offset = read_ue();
    }
    if (sps->vui_parameters_present_flag = GetBits(1))
        InterpretVUI(sps->vui_seq_parameters, cursor);

    cursor = __cursor;
    return sps;
}