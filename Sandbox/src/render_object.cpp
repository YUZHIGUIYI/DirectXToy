//
// Created by ZZK on 2023/5/24.
//

#include <Sandbox/render_object.h>

namespace toy
{
    struct InstanceData
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX world_inv_transpose;
    };

    transform_c& RenderObject::get_transform()
    {
        return m_transform;
    }

    const transform_c& RenderObject::get_transform() const
    {
        return m_transform;
    }

    void RenderObject::frustum_culling(const DirectX::BoundingFrustum &frustum_in_world)
    {
        size_t sz = m_model->meshes.size();
        m_in_frustum = false;
        m_submodel_in_frustum.resize(sz);
        for (size_t i = 0; i < sz; ++i)
        {
            DirectX::BoundingOrientedBox box;
            DirectX::BoundingOrientedBox::CreateFromBoundingBox(box, m_model->meshes[i].bounding_box);
            box.Transform(box, m_transform.get_local_to_world_matrix_xm());
            m_submodel_in_frustum[i] = frustum_in_world.Intersects(box);
            m_in_frustum = m_in_frustum || m_submodel_in_frustum[i];
        }
    }

    void RenderObject::cube_culling(const DirectX::BoundingOrientedBox &obb_in_world)
    {
        size_t sz = m_model->meshes.size();
        m_in_frustum = false;
        m_submodel_in_frustum.resize(sz);
        for (size_t i = 0; i < sz; ++i)
        {
            DirectX::BoundingOrientedBox box;
            DirectX::BoundingOrientedBox::CreateFromBoundingBox(box, m_model->meshes[i].bounding_box);
            box.Transform(box, m_transform.get_local_to_world_matrix_xm());
            m_submodel_in_frustum[i] = obb_in_world.Intersects(box);
            m_in_frustum = m_in_frustum || m_submodel_in_frustum[i];
        }
    }

    void RenderObject::cube_culling(const DirectX::BoundingBox &aabb_in_world)
    {
        size_t sz = m_model->meshes.size();
        m_in_frustum = false;
        m_submodel_in_frustum.resize(sz);
        for (size_t i = 0; i < sz; ++i)
        {
            DirectX::BoundingBox box;
            m_model->meshes[i].bounding_box.Transform(box, m_transform.get_local_to_world_matrix_xm());
            m_submodel_in_frustum[i] = aabb_in_world.Intersects(box);
            m_in_frustum = m_in_frustum || m_submodel_in_frustum[i];
        }
    }

    void RenderObject::set_model(const model::Model *model_)
    {
        m_model = model_;
    }

    const model::Model* RenderObject::get_model() const
    {
        return m_model;
    }

    DirectX::BoundingBox RenderObject::get_local_bounding_box() const
    {
        return m_model ? m_model->bounding_box : DirectX::BoundingBox(DirectX::XMFLOAT3(), DirectX::XMFLOAT3());
    }

    DirectX::BoundingBox RenderObject::get_local_bounding_box(size_t idx) const
    {
        if (!m_model || m_model->meshes.size() >= idx)
        {
            return DirectX::BoundingBox(DirectX::XMFLOAT3(), DirectX::XMFLOAT3());
        }
        return m_model->meshes[idx].bounding_box;
    }

    DirectX::BoundingBox RenderObject::get_bounding_box() const
    {
        if (!m_model)
        {
            return DirectX::BoundingBox(DirectX::XMFLOAT3(), DirectX::XMFLOAT3());
        }
        DirectX::BoundingBox box = m_model->bounding_box;
        box.Transform(box, m_transform.get_local_to_world_matrix_xm());
        return box;
    }

    DirectX::BoundingBox RenderObject::get_bounding_box(size_t idx) const
    {
        if (!m_model || m_model->meshes.size() >= idx)
        {
            return DirectX::BoundingBox(DirectX::XMFLOAT3(), DirectX::XMFLOAT3());
        }
        DirectX::BoundingBox box = m_model->meshes[idx].bounding_box;
        box.Transform(box, m_transform.get_local_to_world_matrix_xm());
        return box;
    }

    DirectX::BoundingOrientedBox RenderObject::get_bounding_oriented_box() const
    {
        if (!m_model)
        {
            return DirectX::BoundingOrientedBox(DirectX::XMFLOAT3(), DirectX::XMFLOAT3(), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
        }
        DirectX::BoundingOrientedBox obb;
        DirectX::BoundingOrientedBox::CreateFromBoundingBox(obb, m_model->bounding_box);
        obb.Transform(obb, m_transform.get_local_to_world_matrix_xm());
        return obb;
    }

    DirectX::BoundingOrientedBox RenderObject::get_bounding_oriented_box(size_t idx) const
    {
        if (!m_model || m_model->meshes.size() >= idx)
        {
            return DirectX::BoundingOrientedBox(DirectX::XMFLOAT3(), DirectX::XMFLOAT3(), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
        }
        DirectX::BoundingOrientedBox obb;
        DirectX::BoundingOrientedBox::CreateFromBoundingBox(obb, m_model->meshes[idx].bounding_box);
        obb.Transform(obb, m_transform.get_local_to_world_matrix_xm());
        return obb;
    }

    void RenderObject::set_visible(bool is_visible)
    {
        m_in_frustum = is_visible;
        m_submodel_in_frustum.assign(m_submodel_in_frustum.size(), true);
    }

    void RenderObject::draw(ID3D11DeviceContext *device_context, IEffect& effect)
    {
        if (!m_in_frustum || !device_context)
        {
            return;
        }

        size_t sz = m_model->meshes.size();
        size_t fsz = m_submodel_in_frustum.size();
        for (size_t i = 0; i < sz; ++i)
        {
            if (i < fsz && !m_submodel_in_frustum[i])
            {
                continue;
            }

            auto* pEffectMeshData = dynamic_cast<IEffectMeshData*>(&effect);
            if (!pEffectMeshData)
            {
                continue;
            }

            auto* pEffectMaterial = dynamic_cast<IEffectMaterial*>(&effect);
            if (pEffectMaterial)
            {
                pEffectMaterial->set_material(m_model->materials[m_model->meshes[i].material_index]);
            }

            auto* pEffectTransform = dynamic_cast<IEffectTransform*>(&effect);
            if (pEffectTransform)
            {
                pEffectTransform->set_world_matrix(m_transform.get_local_to_world_matrix_xm());
            }

            effect.apply(device_context);

            MeshDataInput input = pEffectMeshData->get_input_data(m_model->meshes[i]);
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

    void RenderObject::set_debug_object_name(const std::string &name)
    {

    }
}









