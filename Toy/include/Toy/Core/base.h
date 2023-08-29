//
// Created by ZZK on 2023/5/17.
//

#pragma once

#include <pch.h>
#include <Toy/Core/disable_copyable.h>
#include <Toy/Core/singleton.h>
#include <Toy/Core/logger.h>

#undef max
#undef min

// Enable graphics debug object name
#if (defined(DEBUG) || defined(_DEBUG)) && !defined(GRAPHICS_DEBUGGER_OBJECT_NAME)
    #define GRAPHICS_DEBUGGER_OBJECT_NAME 1
#endif

// Home directory - this should be defined by CMakeLists.txt, that is project root directory
#ifndef DXTOY_HOME
#define DXTOY_HOME
#endif

namespace toy
{
    template<typename T>
    using com_ptr = Microsoft::WRL::ComPtr<T>;

    // Set debug object name
    template<typename IObject>
    void set_debug_object_name(IObject* object, std::string_view name)
    {
        object->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<uint32_t>(name.size()), name.data());
    }
}
