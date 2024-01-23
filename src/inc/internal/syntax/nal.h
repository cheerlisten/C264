#pragma once
#include <c264/c264.h>
#include <syntax/common.h>

enum class NALT
{
    SLICE = 1,               // Coded slice of a non-IDR picture
    DPA = 2,                 // Coded slice data partition A
    DPB = 3,                 // Coded slice data partition B
    DPC = 4,                 // Coded slice data partition C
    IDR = 5,                 // Coded slice of an IDR picture
    SEI = 6,                 // Supplemental enhancement information (SEI)
    SPS = 7,                 // Sequence parameter set
    PPS = 8,                 // Picture parameter set
    AUD = 9,                 // Access unit delimiter
    EOSEQ = 10,              // End of sequence
    EOSTREAM = 11,           // End of stream
    FILL = 12,               // Filler data
    SPSE = 13,               // Sequence parameter set extension
    PREFIX = 14,             // Prefix NAL unit
    SUB_SPS = 15,            // Subset sequence parameter set
    DPS = 16,                // Depth parameter set
                             // 17..18 reserved
    NDP = 19,                // Coded slice of an auxiliary coded picture without partitioning
    SLC_EXT = 20,            // Coded slice extension
    DVC_EXT_OR_3DAVCVC = 21, // Coded slice extension for a depth view component or a 3D-AVC texture view component
    VDRD = 24,               // View and Dependency Representation Delimiter NAL Unit
};

//! values for nal_ref_idc
typedef enum
{
    NALU_PRIORITY_HIGHEST = 3,
    NALU_PRIORITY_HIGH = 2,
    NALU_PRIORITY_LOW = 1,
    NALU_PRIORITY_DISPOSABLE = 0
} NalRefIdc;

// $spec:[G.7.3.1.1] NAL unit header SVC extension syntax
struct nalu_svc_t
{
    int idr_flag;                 // u(1)
    int priority_id;              // u(6)
    int no_inter_layer_pred_flag; // u(1)
    int dependency_id;            // u(3)
    int quality_id;               // u(4)
    int temporal_id;              // u(3)
    int use_ref_base_pic_flag;    // u(1)
    int discardable_flag;         // u(1)
    int output_flag;              // u(1)
    int reserved_three_2bits;     // u(2)
};

// $spec:[H.7.3.1.1] NAL unit header MVC extension syntax
struct nalu_mvc_t
{
    int non_idr_flag;     // u(1)  0 = current is IDR
    int priority_id;      // u(6)  a lower value of priority_id specifies a higher priority
    int view_id;          // u(10) view identifier for the NAL unit
    int temporal_id;      // u(3)  temporal identifier for the NAL unit
    int anchor_pic_flag;  // u(1)  anchor access unit
    int inter_view_flag;  // u(1)  inter-view prediction enable
    int reserved_one_bit; // u(1)  shall be equal to 1
};

// $spec:[J.7.3.1.1] NAL unit header 3D-AVC extension syntax
struct nalu_3davc_t
{
    int view_idx;        // u(8)  specifies the view oder index for the NAL unit.
    int depth_flag;      // u(1)  indicates this extension belongs to [1?depth:texture] view component
    int non_idr_flag;    // u(1)  0 = current is IDR
    int temporal_id;     // u(3)  temporal identifier for the NAL unit
    int anchor_pic_flag; // u(1)  anchor access unit
    int inter_view_flag; // u(1)  inter-view prediction enable
};

// $spec:[7.3.1]
struct nalu_t
{
    int      startcodeprefix_len; // 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
    unsigned len; // Length of the NAL unit (Excluding the start code, which does not belong to the NALU)

    int       forbidden_bit;         // should be always FALSE
    NalRefIdc nal_ref_idc;           // NALU_PRIORITY_xxxx
    NALT      nal_unit_type;         // NALU_TYPE_xxxx
    int       svc_extension_flag;    // should be always 0, for MVC
    int       avc_3d_extension_flag; // should be always 0, for MVC

    nalu_svc_t   opt_svc_extension;
    nalu_mvc_t   opt_mvc_extension;
    nalu_3davc_t opt_3davc_extension;
};

/// @brief range: [begin,end)
/// @return 0:OK
int EBSP2RBSP_Inplace(uint8_t* buffer, int begin, int end);

/// @return 0:OK
int RBSP2SODB_Inplace(uint8_t* buffer, int begin, int end);

/// @brief parse nal
/// @param cursor rbsp data, change only if parse ok, final cursor->bit_pos indicates end of NAL Header
nalu_t* ParseNAL(RBSPCursor& cursor);