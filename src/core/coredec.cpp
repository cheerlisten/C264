#include <core/coredec.h>
#include <syntax/syntax.h>
#include <defines.h>

static void slice_decode_init(Slice_t* slice, RBSPCursor cursor)
{
    auto        dd = &slice->dd;
    const auto* sps = slice->sps;
    const auto* pps = slice->pps;
    bool        ifCabac = pps->entropy_coding_mode_flag;

    dd->MbaffFrameFlag = sps->mb_adaptive_frame_field_flag && !slice->field_pic_flag; // $spec E[7-25]
    dd->PicWidthInMbs = sps->pic_width_in_mbs_minus1 + 1;
    dd->PicHeightInMapUnits = sps->pic_height_in_map_units_minus1 + 1;
    dd->FrameHeightInMbs = (2 - sps->frame_mbs_only_flag) * dd->PicHeightInMapUnits;
    dd->FrameSizeInMbs = dd->PicWidthInMbs * dd->FrameHeightInMbs;
    dd->PicPos.resize(dd->FrameSizeInMbs);
    for (int i = 0; i < dd->FrameSizeInMbs; ++i)
    {
        dd->PicPos[i].x = (short)(i % dd->PicWidthInMbs);
        dd->PicPos[i].y = (short)(i / dd->PicWidthInMbs);
    }
    slice->mb_line_stride = dd->PicWidthInMbs + 1;
    // extreme case when cur == 0: Block D(TopLeft)= - mb_line_stride -1
    // MBAFF need two stride for cache
    slice->mbs_buf.resize(slice->mb_line_stride * (dd->FrameHeightInMbs + 1) + 1);
    slice->mbs = slice->mbs_buf.data() + slice->mb_line_stride + 1;

    // setup decode methods
    switch (slice->slice_type)
    {
    case SliceType::SLICE_I:
        slice->fnDecodeBlock = DecodeBlock_I_CABAC;
        break;
    default:
        AASSERT(!"@todo");
    }

    slice->rbspCursor = cursor;

    if (ifCabac)
    {
        InitContextVariables(slice->cabac_init_idc, dd->SliceQP_Y, slice->cabac.bac_contexts);
        int cabac_offset = (cursor.bit_pos + 0x7) / 8;
        slice->cabac.decoder.Init(cursor.buf + cabac_offset, cursor.bit_length / 8 - cabac_offset);
    }
}

static void update_mb_neibors(Slice_t* slice)
{
    AASSERT(!slice->dd.MbaffFrameFlag);
    int       mbIdx = slice->curMBIdx;
    BlockPos* p_pic_pos = &slice->dd.PicPos[mbIdx];
    slice->mbAddrA = mbIdx - 1;
    slice->mbAddrD = slice->mbAddrA - slice->mb_line_stride;
    slice->mbAddrB = slice->mbAddrD + 1;
    slice->mbAddrC = slice->mbAddrB + 1;

    slice->mbTypeA = slice->mbs[slice->mbAddrA].mb_type;
    slice->mbTypeD = slice->mbs[slice->mbAddrD].mb_type;
    slice->mbTypeB = slice->mbs[slice->mbAddrB].mb_type;
    slice->mbTypeC = slice->mbs[slice->mbAddrC].mb_type;
}

FORCE_INLINE static void start_decode_macroblock(Slice_t* slice, Macroblock_t* curMB)
{
    curMB->slice = slice;
}

static void decode_one_block(Slice_t* slice, Macroblock_t* curMB)
{
    start_decode_macroblock(slice, curMB);
    slice->fnDecodeBlock(curMB);
}

int DecodeSlice(Slice_t* slice, RBSPCursor __cursor)
{
    auto pps = slice->pps;
    auto sps = &slice->sps;
    auto dd = &slice->dd;

    StartBitsCursor(slice->rbspCursor);

    slice_decode_init(slice, __cursor);

    AASSERT(pps->entropy_coding_mode_flag, "@todo CAVLC");
    if (pps->entropy_coding_mode_flag)
    {
        int align_bits = 8 - (slice->rbspCursor.bit_pos & 0b111);
        if (align_bits < 8 && align_bits > 0)
        {
            int bits = GetBits(align_bits);
            AASSERT(bits == (1 << align_bits) - 1, "cabac_alignment_one_bit");
        }
    }

    slice->curMBIdx = slice->first_mb_in_slice * dd->MbaffFrameFlag;

    int moreDataFlag = 1;
    int prevMbSkipped = 0;
    int mb_skip_flag = 0;

    Macroblock_t* curMB = nullptr;
    while (moreDataFlag)
    {
        update_mb_neibors(slice);
        curMB = slice->mbs + slice->curMBIdx;
        memset(curMB, 0, sizeof(*curMB));
        curMB->slice = slice;

        if (slice->slice_type != SliceType::SLICE_I && slice->slice_type != SliceType::SLICE_SI)
            if (!pps->entropy_coding_mode_flag)
            {
                int mb_skip_run = read_ue();
                prevMbSkipped = mb_skip_run > 0;
                for (int i = 0; i < mb_skip_run; ++i)
                    ; //NextMbAddress()
            }
            else
            {
                AASSERT(!"@ae mb_skip_flag");
                moreDataFlag = mb_skip_flag;
            }
        if (moreDataFlag)
        {
            if (dd->MbaffFrameFlag && (slice->curMBIdx & 0x1 == 0 || prevMbSkipped))
                if (!pps->entropy_coding_mode_flag)
                    curMB->mb_field_decoding_flag = GetBits(1);
                else
                    AASSERT(!"@ae curMB->mb_field_decoding_flag");
            decode_one_block(slice, curMB);
        }
        if (!pps->entropy_coding_mode_flag)

            moreDataFlag = MORE_RBSP_DATA();
        else
        {
            if (slice->slice_type != SliceType::SLICE_I && slice->slice_type != SliceType::SLICE_SI)
                prevMbSkipped = mb_skip_flag;
            if (dd->MbaffFrameFlag && slice->curMBIdx % 2 == 0)
                moreDataFlag = 1;
            else
            {
                AASSERT(!"@ae end_of_slice_flag");
            }
        }
        void; //NextMbAddress()
    }
    return 0;
}
