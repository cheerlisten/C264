#include <syntax/nal.h>
#include <log.h>
#include <assert.h>

nalu_t* ParseNAL(RBSPCursor& cursor)
{
    nalu_t* nalu = new nalu_t;
    memset(nalu, 0, sizeof(*nalu));

    RBSPCursor __cursor = cursor;
    StartBitsCursor(__cursor);

    /* default values */
    {
        nalu->svc_extension_flag = -1;
    }

    // start code part is not in shrinked RBSP buffer
    nalu->startcodeprefix_len = cursor.refNal->start_code_length - 1;
    nalu->len = cursor.bit_length / 8;

    nalu->forbidden_bit = GetBits(1);
    nalu->nal_ref_idc = (NalRefIdc)GetBits(2);
    nalu->nal_unit_type = (NALT)GetBits(5);
    if (EqualToAnyOf(nalu->nal_unit_type, NALT::PREFIX, NALT::SLC_EXT, NALT::DVC_EXT_OR_3DAVCVC))
    {
        if (!EqualToAnyOf(nalu->nal_unit_type, NALT::DVC_EXT_OR_3DAVCVC))
            nalu->svc_extension_flag = GetBits(1);
        else
            nalu->avc_3d_extension_flag = GetBits(1);

        if (nalu->svc_extension_flag)
        {
            nalu->opt_svc_extension.idr_flag = GetBits(1);
            nalu->opt_svc_extension.priority_id = GetBits(6);
            nalu->opt_svc_extension.no_inter_layer_pred_flag = GetBits(1);
            nalu->opt_svc_extension.dependency_id = GetBits(3);
            nalu->opt_svc_extension.quality_id = GetBits(4);
            nalu->opt_svc_extension.temporal_id = GetBits(3);
            nalu->opt_svc_extension.use_ref_base_pic_flag = GetBits(1);
            nalu->opt_svc_extension.discardable_flag = GetBits(1);
            nalu->opt_svc_extension.output_flag = GetBits(1);
            nalu->opt_svc_extension.reserved_three_2bits = GetBits(2);
        }
        else if (nalu->avc_3d_extension_flag)
        {
            nalu->opt_3davc_extension.view_idx = GetBits(8);
            nalu->opt_3davc_extension.depth_flag = GetBits(1);
            nalu->opt_3davc_extension.non_idr_flag = GetBits(1);
            nalu->opt_3davc_extension.temporal_id = GetBits(3);
            nalu->opt_3davc_extension.anchor_pic_flag = GetBits(1);
            nalu->opt_3davc_extension.inter_view_flag = GetBits(1);
        }
        else
        {
            nalu->opt_mvc_extension.non_idr_flag = GetBits(1);
            nalu->opt_mvc_extension.priority_id = GetBits(6);
            nalu->opt_mvc_extension.view_id = GetBits(10);
            nalu->opt_mvc_extension.temporal_id = GetBits(3);
            nalu->opt_mvc_extension.anchor_pic_flag = GetBits(1);
            nalu->opt_mvc_extension.inter_view_flag = GetBits(1);
            nalu->opt_mvc_extension.reserved_one_bit = GetBits(1);
        }
    }

    cursor = __cursor;
    return nalu;
}

int EBSP2RBSP_Inplace(uint8_t* buffer, int bp_start, int bp_end)
{
    int i, j, count;
    count = 0;

    if (bp_end < bp_start)
        return bp_end;

    j = bp_start;

    for (i = bp_start; i < bp_end; ++i)
    { //starting from bp_start to avoid header information
        //in NAL unit, 0x000000, 0x000001 or 0x000002 shall not occur at any byte-aligned position
        if (count == 2 && buffer[i] < 0x03)
            return -1;
        if (count == 2 && buffer[i] == 0x03)
        {
            //check the 4th byte after 0x000003, except when cabac_zero_word is used, in which case the last three bytes of this NAL unit must be 0x000003
            if ((i < bp_end - 1) && (buffer[i + 1] > 0x03))
                return -1;
            //if cabac_zero_word is used, the final byte of this NAL unit(0x03) is discarded, and the last two bytes of RBSP must be 0x0000
            if (i == bp_end - 1)
                return j;

            ++i;
            count = 0;
        }
        buffer[j] = buffer[i];
        if (buffer[i] == 0x00)
            ++count;
        else
            count = 0;
        ++j;
    }
    return j;
}

int RBSP2SODB_Inplace(uint8_t* buffer, int bp_start, int bp_end)
{
    int ctr_bit, bitoffset;

    bitoffset = 0;
    //find trailing 1
    ctr_bit = (buffer[bp_end - 1] & (0x01 << bitoffset)); // set up control bit

    while (ctr_bit == 0)
    { // find trailing 1 bit
        ++bitoffset;
        if (bitoffset == 8)
        {
            if (bp_end == 0)
                LOG(LL_Panic, " Panic: All zero data sequence in RBSP \n");
            AASSERT(bp_end != 0);
            --bp_end;
            bitoffset = 0;
        }
        ctr_bit = buffer[bp_end - 1] & (0x01 << (bitoffset));
    }
    return (bp_end);
}