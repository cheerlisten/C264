#pragma once
#include <memory>
#include <vector>
#include <c264/c264.h>
#include <bitGetter.h>
#include <syntax/common.h>
#include <syntax/macroblock.h>
#include <syntax/nal.h>
#include <syntax/parset.h>
#include <syntax/slice.h>

struct Parser
{
    /// @brief nal parse entry
    /// @param nalBufView bitstream data: startcode + NALU + RBSP trailling bits
    int SyntaxParse(const BufferView* nalBufView);

    struct Sema* sema;

    std::vector<std::unique_ptr<Seq_parameter_set_rbsp_t>> sps;
    std::vector<std::unique_ptr<pic_parameter_set_rbsp_t>> pps;
};