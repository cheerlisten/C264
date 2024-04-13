#include <syntax/syntax.h>
#include <math.h>
#include <defines.h>
#include <log.h>

static void ref_pic_list_reordering(Slice_t* slice, RBSPCursor& cursor)
{
    StartBitsCursor(cursor);
    const seq_parameter_set_rbsp_t* sps = slice->sps;
    int                             i, val;

    //   alloc_ref_pic_list_reordering_buffer(slice);
    for (int listIdx = 0; listIdx < LIST_CNT; ++listIdx)
        if ((listIdx == 0 && (slice->slice_type != SLICE_I && slice->slice_type != SLICE_SI)) ||
            (listIdx == 1 && (slice->slice_type == SLICE_B)))
        {
            slice->ref_pic_list_modification_flag[LIST_0] = GetBits(1);

            if (slice->ref_pic_list_modification_flag)
            {
                int i = 0, idc;
                do
                {
                    idc = slice->modification_of_pic_nums_idc[LIST_0][i] = read_ue();
                    if (idc == 0 || idc == 1)
                        slice->abs_diff_pic_num_minus1[LIST_0][i] = read_ue();
                    else if (idc == 2)
                        slice->long_term_pic_num[LIST_0][i] = read_ue();
                    i++;
                } while (idc != 3);
            }
        }

    // note: ignore redundant picture
    // // set reference index of redundant slices.
    // if (slice->redundant_pic_cnt && (slice->slice_type != SLICE_I))
    // {
    //     slice->redundant_slice_ref_idx = slice->refPicListReorders[LIST_0].abs_diff_pic_num_minus1[0] + 1;
    // }
}

static void pred_weight_table(Slice_t* slice, RBSPCursor& cursor)
{
    StartBitsCursor(cursor);

    const seq_parameter_set_rbsp_t* sps = slice->sps;
    const pic_parameter_set_rbsp_t* pps = slice->pps;

    slice->luma_log2_weight_denom = read_ue();

    if (sps->chroma_format_idc != 0)
        slice->chroma_log2_weight_denom = (unsigned short)read_ue();

    for (int listIdx = 0; listIdx < LIST_CNT; ++listIdx)
    {
        if (listIdx == 1 && slice->slice_type != SLICE_B)
            continue;
        for (int i = 0; i < slice->num_ref_idx_active_minus1[listIdx]; i++)
        {
            slice->luma_weight_flag[listIdx] = GetBits(1);

            if (slice->luma_weight_flag[listIdx])
            {
                slice->wp_weight[listIdx][i][0] = read_se();
                slice->wp_offset[listIdx][i][0] = read_se();
                slice->wp_offset[listIdx][i][0] = slice->wp_offset[listIdx][i][0] << (sps->bit_depth_luma_minus8);
            }
            else
            {
                slice->wp_weight[listIdx][i][0] = 1 << slice->luma_log2_weight_denom;
                slice->wp_offset[listIdx][i][0] = 0;
            }

            if (sps->chroma_format_idc != 0)
            {
                slice->chroma_weight_flag[listIdx] = GetBits(1);

                for (int j = 1; j < 3; j++)
                {
                    if (slice->chroma_weight_flag[listIdx])
                    {
                        slice->wp_weight[listIdx][i][j] = read_se();
                        slice->wp_offset[listIdx][i][j] = read_se();
                        slice->wp_offset[listIdx][i][j] = slice->wp_offset[listIdx][i][j]
                                                          << (sps->bit_depth_chroma_minus8);
                    }
                    else
                    {
                        slice->wp_weight[listIdx][i][j] = 1 << slice->chroma_log2_weight_denom;
                        slice->wp_offset[listIdx][i][j] = 0;
                    }
                }
            }
        }
    }
}

void dec_ref_pic_marking(Slice_t* slice, RBSPCursor& cursor)
{
    StartBitsCursor(cursor);
    const nalu_t* nalu = slice->nalu;

    if (slice->nalu->nal_unit_type == NALT::IDR)

#if (MVC_EXTENSION_ENABLE)
        if (nalu->nal_unit_type == NALT::IDR ||
            (nalu->svc_extension_flag == 0 && nalu->opt_svc_extension.non_idr_flag == 0))
#else
        if (nalu->nal_unit_type == NALT::IDR)
#endif
        {
            slice->no_output_of_prior_pics_flag = GetBits(1);
            slice->long_term_reference_flag = GetBits(1);
        }
        else
        {
            slice->adaptive_ref_pic_buffering_flag = GetBits(1);
            if (slice->adaptive_ref_pic_buffering_flag)
            {
                int mmco = -1;
                for (int idx = 0; mmco != 0; ++idx)
                {
                    AASSERT(idx < MAX_MMCO_COUNT);
                    slice_t::decRefPicMarking_t* decRefPicMarking = slice->decRefPicMarking + idx;
                    mmco = decRefPicMarking->memory_management_control_operation = read_ue();

                    if ((mmco == 1) || (mmco == 3))
                        decRefPicMarking->difference_of_pic_nums_minus1 = read_ue();
                    if (mmco == 2)
                        decRefPicMarking->long_term_pic_num = read_ue();

                    if ((mmco == 3) || (mmco == 6))
                        decRefPicMarking->long_term_frame_idx = read_ue();
                    if (mmco == 4)
                        decRefPicMarking->max_long_term_frame_idx_plus1 = read_ue();
                }
            }
        }
}

std::unique_ptr<Slice_t> ParseSliceHeader(RBSPCursor& cursor, const struct Parser* parser, const nalu_t* nalu)
{
    std::unique_ptr<Slice_t> slice = std::make_unique<Slice_t>();
    auto                     deduce = &slice->dd;

    // nal info
    deduce->dp_mode = nalu->nal_unit_type == NALT::IDR ? Dp_Mode::DP_NONE : Dp_Mode(int(nalu->nal_unit_type));
    slice->nalu = nalu;

    RBSPCursor __cursor = cursor;
    StartBitsCursor(__cursor);

    slice->first_mb_in_slice = read_ue();

    slice->slice_type = (SliceType)read_ue();
    if (slice->slice_type > 4)
        slice->slice_type = (SliceType)(slice->slice_type - 5);

    slice->pic_parameter_set_id = read_ue();

    if (parser->pps.size() < slice->pic_parameter_set_id)
        return nullptr;
    const pic_parameter_set_rbsp_t* pps = parser->pps[slice->pic_parameter_set_id].get();
    slice->sps = parser->sps[pps->seq_parameter_set_id].get();
    const seq_parameter_set_rbsp_t* sps = slice->sps;
    slice->pps = pps;

    // PS infos
    deduce->QpBdOffsetY = 6 * sps->bit_depth_luma_minus8; // $spec E[7-4]

    if (sps->separate_colour_plane_flag)
        slice->colour_plane_id = GetBits(2);
    else
        slice->colour_plane_id = PLANE_Y;

    slice->frame_num = GetBits(sps->log2_max_frame_num_minus4 + 4);

    if (!sps->frame_mbs_only_flag)
    {
        slice->field_pic_flag = GetBits(1);
        if (slice->field_pic_flag)
            slice->bottom_field_flag = GetBits(1);
    }

    // $spec[Equation:7-1]
    bool IdrPicFlag = nalu->nal_unit_type == NALT::IDR;
    if (IdrPicFlag)
        slice->idr_pic_id = read_ue();

    if (sps->pic_order_cnt_type == 0)
    {
        slice->pic_order_cnt_lsb = GetBits(sps->log2_max_pic_order_cnt_lsb_minus4 + 4);
        if (pps->bottom_field_pic_order_in_frame_present_flag == 1 && !slice->field_pic_flag)
            slice->delta_pic_order_cnt_bottom = read_se();
        else
            slice->delta_pic_order_cnt_bottom = 0;
    }
    else if (sps->pic_order_cnt_type == 1)
    {
        if (!sps->delta_pic_order_always_zero_flag)
        {
            slice->delta_pic_order_cnt[0] = read_se();
            if (pps->bottom_field_pic_order_in_frame_present_flag == 1 && !slice->field_pic_flag)
                slice->delta_pic_order_cnt[1] = read_se();
            else
                slice->delta_pic_order_cnt[1] = 0;
        }
        else
        {
            slice->delta_pic_order_cnt[0] = 0;
            slice->delta_pic_order_cnt[1] = 0;
        }
    }

    //! redundant_pic_cnt is missing here
    if (pps->redundant_pic_cnt_present_flag)
    {
        slice->redundant_pic_cnt = read_ue();
    }

    if (slice->slice_type == SLICE_B)
    {
        slice->direct_spatial_mv_pred_flag = GetBits(1);
    }

    { /* num_ref_idx */

        slice->num_ref_idx_active_minus1[LIST_0] = pps->num_ref_idx_l0_default_active_minus1 + 1;
        slice->num_ref_idx_active_minus1[LIST_1] = pps->num_ref_idx_l1_default_active_minus1 + 1;

        if (slice->slice_type == SLICE_P || slice->slice_type == SLICE_SP || slice->slice_type == SLICE_B)
        {
            slice->num_ref_idx_active_override_flag = GetBits(1);
            if (slice->num_ref_idx_active_override_flag)
            {
                slice->num_ref_idx_active_minus1[LIST_0] = 1 + read_ue();

                if (slice->slice_type == SLICE_B)
                {
                    slice->num_ref_idx_active_minus1[LIST_1] = 1 + read_ue();
                }
            }
        }
        if (slice->slice_type != SLICE_B)
        {
            slice->num_ref_idx_active_minus1[LIST_1] = 0;
        }
        if ((slice->slice_type == SLICE_P || slice->slice_type == SLICE_SP || slice->slice_type == SLICE_B) &&
            !slice->field_pic_flag && pps->num_ref_idx_l0_default_active_minus1 > 15)
            AASSERT(slice->num_ref_idx_active_override_flag);
        if (slice->slice_type == SLICE_B && !slice->field_pic_flag && pps->num_ref_idx_l1_default_active_minus1 > 15)
            AASSERT(slice->num_ref_idx_active_override_flag);
    }

    if (nalu->nal_unit_type == NALT::SLC_EXT || nalu->nal_unit_type == NALT::DVC_EXT_OR_3DAVCVC)
        AASSERT(!"mvc not supported"); // ref_pic_list_mvc_modification(slice); // specified in AnnexH
    else
        ref_pic_list_reordering(slice.get(), __cursor); //ref_pic_list_reordering(slice);

    // slice->weighted_pred_flag = (unsigned short)((slice->slice_type == SLICE_P || slice->slice_type == SLICE_SP) ?
    //                                                  pps->weighted_pred_flag :
    //                                                  (slice->slice_type == SLICE_B && pps->weighted_bipred_idc == 1));
    // slice->weighted_bipred_idc = (unsigned short)(slice->slice_type == SLICE_B && pps->weighted_bipred_idc > 0);

    if ((pps->weighted_pred_flag && (slice->slice_type == SLICE_P || slice->slice_type == SLICE_SP)) ||
        (pps->weighted_bipred_idc == 1 && (slice->slice_type == SLICE_B)))
    {
        pred_weight_table(slice.get(), __cursor);
    }

    if (nalu->nal_ref_idc)
        dec_ref_pic_marking(slice.get(), __cursor);

    if (pps->entropy_coding_mode_flag && slice->slice_type != SLICE_I && slice->slice_type != SLICE_SI)
        slice->cabac_init_idc = read_ue();
    else
        slice->cabac_init_idc = 0;

    slice->slice_qp_delta = read_se();
    deduce->SliceQP_Y = 26 + pps->pic_init_qp_minus26 + slice->slice_qp_delta; // $spec S[] slice_qp_delta
    AASSERT((deduce->SliceQP_Y >= -deduce->SliceQP_Y) && (deduce->SliceQP_Y <= 51),
            "slice_qp_delta=%d makes SliceQP_Y out of range",
            deduce->SliceQP_Y);

    if (slice->slice_type == SLICE_SP || slice->slice_type == SLICE_SI)
    {
        if (slice->slice_type == SLICE_SP)
        {
            slice->sp_for_switch_flag = GetBits(1);
        }
        slice->slice_qs_delta = read_se();
        deduce->QS_Y = 26 + pps->pic_init_qs_minus26 + slice->slice_qs_delta;
        AASSERT(deduce->QS_Y >= 0 && deduce->QS_Y <= 51,
                "slice_qs_delta=%d makes QS_Y=%d out of range",
                slice->slice_qs_delta,
                deduce->QS_Y);
    }

    if (pps->deblocking_filter_control_present_flag)
    {
        slice->disable_deblocking_filter_idc = (short)read_ue();

        if (slice->disable_deblocking_filter_idc != 1)
        {
            slice->slice_alpha_c0_offset_div2 = read_se();
            slice->slice_beta_offset_div2 = read_se();
            deduce->FilterOffsetA = slice->slice_alpha_c0_offset_div2 * 2;
            deduce->FilterOffsetB = slice->slice_beta_offset_div2 * 2;
        }
    }

    if (pps->num_slice_groups_minus1 > 0 && pps->slice_group_map_type >= 3 && pps->slice_group_map_type <= 5)
    {
        // $spec E[7-17]
        int PicSizeInMapUnits = (sps->pic_height_in_map_units_minus1 + 1) * (sps->pic_width_in_mbs_minus1 + 1);
        int bits = log2(PicSizeInMapUnits / (pps->slice_group_change_rate_minus1 + 1) + 1);
        slice->slice_group_change_cycle = GetBits(bits);
    }

    cursor = __cursor;
    return slice;
}

Slice_t::~Slice_t()
{
    mbs_buf.clear();
    mbs = nullptr;
    mb_line_stride = 0;
}
