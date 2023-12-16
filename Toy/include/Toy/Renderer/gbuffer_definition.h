//
// Created by ZZK on 2023/11/27.
//

#pragma once

#include <Toy/Renderer/texture_2d.h>

namespace toy
{
    struct GBufferDefinition
    {
        // Albedo and metalness
        std::unique_ptr<Texture2D> albedo_metalness_buffer = nullptr;
        // Normal and roughness
        std::unique_ptr<Texture2D> normal_roughness_buffer = nullptr;
        // World position
        std::unique_ptr<Texture2D> world_position_buffer = nullptr;
        // Motion vector
        std::unique_ptr<Texture2D> motion_vector_buffer = nullptr;
        // Entity id
        std::unique_ptr<Texture2D> entity_id_buffer = nullptr;
    };
}
