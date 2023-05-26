//
// Created by ZZK on 2023/5/17.
//

#pragma once

#include <pch.h>
#include <Toy/Core/disable_copyable.h>
#include <Toy/Core/singleton.h>
#include <Toy/Core/logger.h>

// Enable graphics debug
#ifndef GRAPHICS_DEBUGGER_OBJECT_NAME
    #define GRAPHICS_DEBUGGER_OBJECT_NAME (1)
#endif

namespace toy
{
    template<typename T>
    using com_ptr = Microsoft::WRL::ComPtr<T>;

    // d3d11_set_debug_object_name function
    // Set object name for object created by d3d device
    // resource - object created by d3d11 device
    // name     - object name
    template<uint32_t Length>
    void d3d11_set_debug_object_name(_In_ ID3D11DeviceChild* resource, _In_ const char(&name)[Length])
    {
#if (defined(_DEBUG) && (GRAPHICS_DEBUGGER_OBJECT_NAME))
        resource->SetPrivateData(WKPDID_D3DDebugObjectName, Length - 1, name);
#else
        UNREFERENCED_PARAMETER(resource);
        UNREFERENCED_PARAMETER(name);
        UNREFERENCED_PARAMETER(length);
#endif
    }

    // dxgi_set_debug_object_name function
    // Set object name for DXGI object
    // object - DXGI object
    // name   - object name
    template<uint32_t Length>
    void dxgi_set_debug_object_name(_In_ IDXGIObject* object, _In_ const char(&name)[Length])
    {
#if (defined(_DEBUG) && (GRAPHICS_DEBUGGER_OBJECT_NAME))
        object->SetPrivateData(WKPDID_D3DDebugObjectName, Length - 1, name);
#else
        UNREFERENCED_PARAMETER(object);
        UNREFERENCED_PARAMETER(name);
#endif
    }
}
