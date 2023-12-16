//
// Created by ZZK on 2023/12/4.
//

#pragma once

#include <Sandbox/viewer_specification.h>

namespace toy
{
    struct Gizmos
    {
    public:
        explicit Gizmos(D3DApplication *d3d_application);

        void on_gizmos_render(Entity &selected_entity, const ViewerSpecification &viewer_specification, std::shared_ptr<camera_c> camera);

    private:
        D3DApplication *d3d_app = nullptr;
    };
}
