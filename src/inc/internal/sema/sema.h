#pragma once
#include <c264/c264.h>
#include <syntax/syntax.h>

struct Sema
{
    int EmitSPS(nalu_t* nalu, std::unique_ptr<Seq_parameter_set_rbsp_t>& sps);
    int EmitPPS(nalu_t* nalu, std::unique_ptr<pic_parameter_set_rbsp_t>& pps);
};