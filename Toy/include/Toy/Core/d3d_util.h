//
// Created by ZZK on 2023/5/18.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{
    // ------------------------------
    // create_shader_from_file函数
    // ------------------------------
    // [In]cso_file_name    编译好的着色器二进制文件(.cso)，若有指定则优先寻找该文件并读取
    // [In]hlsl_file_name   着色器代码，若未找到着色器二进制文件则编译着色器代码
    // [In]entry_point      入口点(指定开始的函数)
    // [In]shader_model     着色器模型，格式为"*s_5_0"，*可以为c,d,g,h,p,v之一
    // [Out]blob_out_pp     输出着色器二进制信息
    HRESULT create_shader_from_file(const wchar_t* cso_file_name, const wchar_t* hlsl_file_name, const char* entry_point,
                                    const char* shader_model, ID3DBlob** blob_out_pp);
}
