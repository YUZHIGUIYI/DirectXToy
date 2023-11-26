//
// Created by ZZK on 2023/11/25.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{
    enum class ShadowType : uint8_t
    {
        ShadowType_CSM,
        ShadowType_VSM,
        ShadowType_ESM,
        ShadowType_EVSM2,
        ShadowType_EVSM4
    };

    enum class CascadeSelection : uint8_t
    {
        CascadeSelection_Map,
        CascadeSelection_Interval
    };

    enum class CameraSelection : uint8_t
    {
        CameraSelection_Eye,
        CameraSelection_Light,
        CameraSelection_Cascade1,
        CameraSelection_Cascade2,
        CameraSelection_Cascade3,
        CameraSelection_Cascade4,
        CameraSelection_Cascade5,
        CameraSelection_Cascade6,
        CameraSelection_Cascade7,
        CameraSelection_Cascade8,
    };

    enum class FitNearFar : uint8_t
    {
        FitNearFar_ZeroOne,
        FitNearFar_CascadeAABB,
        FitNearFar_SceneAABB,
        FitNearFar_SceneAABB_Intersection
    };

    enum class FitProjection : uint8_t
    {
        FitProjection_ToCascade,
        FitProjection_ToScene
    };
}
