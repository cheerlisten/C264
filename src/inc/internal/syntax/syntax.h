#pragma once
#include <memory>
#include <vector>
#include <c264/c264.h>
#include <bitGetter.h>
#include <syntax/common.h>
#include <syntax/nal.h>
#include <syntax/parset.h>

struct Parser
{
    /// @brief nal parse entry
    /// @param nalBufView bitstream data: startcode + NALU + RBSP trailling bits
    int SyntaxParse(const BufferView* nalBufView);

    struct Sema* sema;

    std::vector<std::unique_ptr<seq_parameter_set_rbsp_t>> sps;
};