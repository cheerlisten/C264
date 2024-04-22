#include <syntax/syntax.h>
#include <log.h>

// Table 6-1 –SubWidthC, and SubHeightC values derived from
// chroma_format_idc and separate_colour_plane_flag
const ChromaFormatInfo g_chromaFormatInfos[4]{
    { ChormaFormatIDC::Monochronome, -1, -1 },
    { ChormaFormatIDC::XYZ420, 2, 2 },
    { ChormaFormatIDC::XYZ422, 2, 1 },
    { ChormaFormatIDC::XYZ444, 1, 1 },
};

static void Scaling_List(int* scalingList, int sizeOfScalingList, Bool* useDefaultScalingMatrix, RBSPCursor& cursor)
{
    StartBitsCursor(cursor);
    int lastScale = 8;
    int nextScale = 8;
    for (int j = 0; j < sizeOfScalingList; ++j)
    {
        // $spec[8.5.6] + $spec[8.5.7] zig-zag scan
        int scanj = (sizeOfScalingList == 16) ? g_kuiZigzagScan[j] : g_kuiZigzagScan8x8[j];
        if (nextScale != 0)
        {
            int delta_scale = read_se();
            nextScale = (lastScale + delta_scale + 256) % 256;
            *useDefaultScalingMatrix = (scanj == 0 && nextScale == 0);
        }
        scalingList[scanj] = (nextScale == 0) ? lastScale : nextScale;
        lastScale = scalingList[scanj];
    }
}

static void InterpretHRD(hrd_parameters_t& hrd, RBSPCursor& cursor)
{
    StartBitsCursor(cursor);

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

    StartBitsCursor(cursor);

    if (vui.aspect_ratio_info_present_flag = GetBits(1))
    {
        vui.aspect_ratio_idc = GetBits(8);
        /* $spec Table[E-1] Extended_SAR=255 */
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

std::unique_ptr<Seq_parameter_set_rbsp_t> ParseSPS(RBSPCursor& cursor)
{
    std::unique_ptr<Seq_parameter_set_rbsp_t> sps = std::make_unique<Seq_parameter_set_rbsp_t>();

    auto* dd = &sps->dd;

    RBSPCursor __cursor = cursor;
    StartBitsCursor(__cursor);

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
        sps->qpprime_y_zero_transform_bypass_flag = GetBits(1);
        sps->seq_scaling_matrix_present_flag = GetBits(1);
        if (sps->seq_scaling_matrix_present_flag)
        {
            for (int i = 0; i < (sps->chroma_format_idc != 3 ? 8 : 12); i++)
            {
                sps->seq_scaling_list_present_flag[i] = GetBits(1);
                if (sps->seq_scaling_list_present_flag[i])
                {
                    if (i < 6)
                        Scaling_List(sps->ScalingList4x4[i], 16, &sps->UseDefaultScalingMatrix4x4Flag[i], __cursor);
                    else
                        Scaling_List(sps->ScalingList4x4[i], 64, &sps->UseDefaultScalingMatrix4x4Flag[i - 6], __cursor);
                }
            }
        }
    }
    else
    { // $spec[7.4.2] default values
        sps->chroma_format_idc = 1;
        sps->bit_depth_luma_minus8 = 0;
        sps->bit_depth_chroma_minus8 = 0;
        sps->qpprime_y_zero_transform_bypass_flag = 0;
        sps->separate_colour_plane_flag = 0;
    }
    AASSERT(sps->bit_depth_chroma_minus8 == sps->bit_depth_chroma_minus8, "not support diff vals yet");

    // $spec E7-3 E7-4 E7-5 E7-6
    dd->BitDepth_Y = 8 + sps->bit_depth_luma_minus8;
    dd->QpBdOffset_Y = 6 * sps->bit_depth_luma_minus8;
    dd->BitDepth_C = 8 + sps->bit_depth_chroma_minus8;
    dd->QpBdOffset_C = 6 * sps->bit_depth_chroma_minus8;
    // $spec E6-1 E6-2
    auto& chromaInfo = g_chromaFormatInfos[sps->chroma_format_idc];
    dd->MbHeightC = 16 / chromaInfo.SubHeightC;
    dd->MbWidthC = 16 / chromaInfo.SubWidthC;

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
        InterpretVUI(sps->vui_seq_parameters, __cursor);

    cursor = __cursor;
    return sps;
}

std::unique_ptr<pic_parameter_set_rbsp_t> ParsePPS(RBSPCursor& cursor, const struct Parser* parser)
{
    std::unique_ptr<pic_parameter_set_rbsp_t> pps = std::make_unique<pic_parameter_set_rbsp_t>();

    RBSPCursor __cursor = cursor;
    StartBitsCursor(__cursor);

    pps->pic_parameter_set_id = read_ue();
    pps->seq_parameter_set_id = read_ue();
    pps->entropy_coding_mode_flag = GetBits(1);
    pps->bottom_field_pic_order_in_frame_present_flag = GetBits(1);
    pps->num_slice_groups_minus1 = read_ue();

    if (pps->num_slice_groups_minus1 > 0)
    {
        pps->slice_group_map_type = read_ue();

        if (pps->slice_group_map_type == 0)
        {
            for (int i = 0; i <= pps->slice_group_map_type; ++i)
                pps->run_length_minus1[i] = read_ue();
        }
        else if (pps->slice_group_map_type == 2)
        {
            for (int i = 0; i <= pps->slice_group_map_type; ++i)
            {
                pps->top_left[i] = read_ue();
                pps->bottom_right[i] = read_ue();
            }
        }
        else if (pps->slice_group_map_type == 3 || pps->slice_group_map_type == 4 || pps->slice_group_map_type == 5)
        {
            pps->slice_group_change_direction_flag = GetBits(1);
            pps->slice_group_change_rate_minus1 = read_ue();
        }
        else if (pps->slice_group_map_type == 6)
        {
            pps->pic_size_in_map_units_minus1 = read_ue();
            for (int i = 0; i <= pps->pic_size_in_map_units_minus1; ++i)
                pps->slice_group_id[i] = read_ue();
        }
    }
    pps->num_ref_idx_l0_default_active_minus1 = read_ue();
    pps->num_ref_idx_l1_default_active_minus1 = read_ue();
    pps->weighted_pred_flag = GetBits(1);
    pps->weighted_bipred_idc = GetBits(2);
    pps->pic_init_qp_minus26 = read_se();
    pps->pic_init_qs_minus26 = read_se();
    pps->chroma_qp_index_offset = read_se();
    pps->deblocking_filter_control_present_flag = GetBits(1);
    pps->constrained_intra_pred_flag = GetBits(1);
    pps->redundant_pic_cnt_present_flag = GetBits(1);

    if (MORE_RBSP_DATA())
    {
        pps->transform_8x8_mode_flag = GetBits(1);
        pps->pic_scaling_matrix_present_flag = GetBits(1);
        if (pps->pic_scaling_matrix_present_flag)
        {
            if (!parser || parser->sps.size() <= pps->seq_parameter_set_id)
            {
                LOG(LL_Error,
                    "missing or wrong PPS->seq_parameter_set_id=%d, such SPS(cur got %d SPS) is not found, exit",
                    pps->seq_parameter_set_id,
                    parser ? parser->sps.size() : 0);
                return nullptr;
            }
            seq_parameter_set_rbsp_t* sps = parser->sps[pps->seq_parameter_set_id].get();
            int flags_count = 6 + (sps->chroma_format_idc != 3 ? 2 : 6) * pps->transform_8x8_mode_flag;
            for (int i = 0; i < flags_count; ++i)
            {
                pps->pic_scaling_list_present_flag[i] = GetBits(1);
                if (pps->pic_scaling_list_present_flag[i])
                {
                    if (i < 6)
                        if (i < 6)
                            Scaling_List(sps->ScalingList4x4[i], 16, &sps->UseDefaultScalingMatrix4x4Flag[i], __cursor);
                        else
                            Scaling_List(
                                sps->ScalingList4x4[i], 64, &sps->UseDefaultScalingMatrix4x4Flag[i - 6], __cursor);
                }
            }
        }
        pps->second_chroma_qp_index_offset = read_se();
    }
    else
    { // $spec.said When second_chroma_qp_index_offset is not present, it shall be inferred to be equal to chroma_qp_index_offset.
        pps->second_chroma_qp_index_offset = pps->chroma_qp_index_offset;
    }

    cursor = __cursor;
    return pps;
}