//
// Created by ZZK on 2023/7/2.
//

#include <Toy/Scene/components.h>
#include <Toy/Model/model_manager.h>

namespace toy
{
    void StaticMeshComponent::frustum_culling(const toy::transform_c &transform, const DirectX::BoundingFrustum &frustum_in_world)
    {
        size_t sz = model_asset->meshes.size();
        in_frustum = false;
        submodel_in_frustum.resize(sz);
        for (size_t i = 0; i < sz; ++i)
        {
            DirectX::BoundingOrientedBox box{};
            DirectX::BoundingOrientedBox::CreateFromBoundingBox(box, model_asset->meshes[i].bounding_box);
            box.Transform(box, transform.get_local_to_world_matrix_xm());
            submodel_in_frustum[i] = frustum_in_world.Intersects(box);
            in_frustum = in_frustum || submodel_in_frustum[i];
        }
    }

    void StaticMeshComponent::render(ID3D11DeviceContext *device_context, toy::IEffect &effect, const toy::transform_c &transform)
    {
        if (!in_frustum || !device_context)
        {
            return;
        }

        size_t sz = model_asset->meshes.size();
        size_t fsz = submodel_in_frustum.size();
        for (size_t i = 0; i < sz; ++i)
        {
            if (i < fsz && !submodel_in_frustum[i])
            {
                continue;
            }

            auto* pEffectMeshData = dynamic_cast<IEffectMeshData *>(&effect);
            if (!pEffectMeshData)
            {
                continue;
            }

            auto* pEffectMaterial = dynamic_cast<IEffectMaterial *>(&effect);
            if (pEffectMaterial)
            {
                pEffectMaterial->set_material(model_asset->materials[model_asset->meshes[i].material_index]);
            }

            auto* pEffectTransform = dynamic_cast<IEffectTransform *>(&effect);
            if (pEffectTransform)
            {
                pEffectTransform->set_world_matrix(transform.get_local_to_world_matrix_xm());
            }

            effect.apply(device_context);

            MeshDataInput input = pEffectMeshData->get_input_data(model_asset->meshes[i]);
            {
                device_context->IASetInputLayout(input.input_layout);
                device_context->IASetPrimitiveTopology(input.topology);
                device_context->IASetVertexBuffers(0, (uint32_t)input.vertex_buffers.size(),
                                                    input.vertex_buffers.data(), input.strides.data(), input.offsets.data());
                device_context->IASetIndexBuffer(input.index_buffer, input.index_count > 65535 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT, 0);
                device_context->DrawIndexed(input.index_count, 0, 0);
            }
        }
    }
}