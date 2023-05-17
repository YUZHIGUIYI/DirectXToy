//
// Created by ZZK on 2023/5/17.
//

#pragma once

namespace toy
{
    class disable_copyable_c
    {
    public:
        disable_copyable_c() = default;

        disable_copyable_c(const disable_copyable_c &) = delete;
        disable_copyable_c& operator=(const disable_copyable_c &) = delete;

        // unnecessary definition
        disable_copyable_c(disable_copyable_c &&) = delete;
        disable_copyable_c& operator=(disable_copyable_c &&) = delete;
    };
}
