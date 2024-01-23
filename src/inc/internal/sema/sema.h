#pragma once
#include <c264/c264.h>
#include <syntax/syntax.h>

struct Sema
{
    int EmitSPS(nalu_t* nalu, seq_parameter_set_rbsp_t* sps);
};