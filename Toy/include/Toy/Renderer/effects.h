//
// Created by ZHIKANG on 2023/5/27.
//

#pragma once

#include <Toy/Renderer/render_states.h>
#include <Toy/Renderer/effect_helper.h>
#include <Toy/Renderer/constant_data.h>

namespace toy
{
    class effect_interface_c
    {
    public:
        effect_interface_c() = default;
        virtual ~effect_interface_c() = default;

        // Prohibit copy, allow move
        effect_interface_c(const effect_interface_c&) = delete;
        effect_interface_c& operator=(const effect_interface_c&) = delete;

        effect_interface_c(effect_interface_c&&) = default;
        effect_interface_c& operator=(effect_interface_c&&) = default;

        // Update binding constant buffers
        virtual void apply(ID3D11DeviceContext *device_context) = 0;
    };

    using IEffect = effect_interface_c;

    // As a singleton
    class basic_effect_c : public effect_interface_c
    {
    public:
        basic_effect_c();
        ~basic_effect_c() override = default;

        basic_effect_c(basic_effect_c&& other) noexcept;
        basic_effect_c& operator=(basic_effect_c&& other) noexcept;

    public:
        // Initialize all resources
        void init(ID3D11Device *device);

        // Change render mode
        // Render with default mode
        void set_default_render(ID3D11DeviceContext *device_context);
        // Render with alpha blending
        void set_alpha_blend_render(ID3D11DeviceContext *device_context);
        // Render with close depth test , for effect, use additive blend
        void set_no_depth_test_render(ID3D11DeviceContext *device_context);
        // Render with close depth write, for effect, use additive blend
        void set_no_depth_write_render(ID3D11DeviceContext *device_context);
        // Render with no-double blending
        void set_no_double_blend_render(ID3D11DeviceContext *device_context, uint32_t stencil_ref);
        // Render with write-stencil only
        void set_write_stencil_only_render(ID3D11DeviceContext *device_context, uint32_t stencil_ref);
        // Render only specified stencil value area, use default state
        void set_default_with_stencil_render(ID3D11DeviceContext *device_context, uint32_t stencil_ref);
        // Render only specified stencil value area, use alpha blending
        void set_alpha_blend_with_stencil_render(ID3D11DeviceContext *device_context, uint32_t stencil_ref);
        // Render with close depth test, only specified stencil value area, use additive blend
        void set_no_depth_test_with_stencil_render(ID3D11DeviceContext *device_context, uint32_t stencil_ref);
        // Render with close depth write, only specified stencil value area, use additive blend
        void set_no_depth_write_with_stencil_render(ID3D11DeviceContext * device_context, uint32_t stencil_ref);
        // 2D default state render
        void set_2d_default_render(ID3D11DeviceContext *device_context);
        // 2D alpha blend render
        void set_2d_alpha_blend_render(ID3D11DeviceContext *device_context);

        // Set matrix
        void XM_CALLCONV set_world_matrix(DirectX::FXMMATRIX world);
        void XM_CALLCONV set_view_matrix(DirectX::FXMMATRIX view);
        void XM_CALLCONV set_proj_matrix(DirectX::FXMMATRIX proj);

        void XM_CALLCONV set_reflection_matrix(DirectX::FXMMATRIX reflection);
        void XM_CALLCONV set_shadow_matrix(DirectX::FXMMATRIX shadow);
        void XM_CALLCONV set_ref_shadow_matrix(DirectX::FXMMATRIX ref_shadow);

        // Lighting, material and texture
        void set_dir_light(size_t pos, const DirectionalLight& dir_light);
        void set_point_light(size_t pos, const PointLight& point_light);

        void set_material(const Material& material);

        void set_texture(ID3D11ShaderResourceView *texture);
        void set_eye_pos(const DirectX::XMFLOAT3& eye_pos);

        // Change state
        void set_reflection_state(bool state);
        void set_shadow_state(bool state);

        // Apply changes to constant buffers and texture resources
        void apply(ID3D11DeviceContext *device_context) override;

    private:
        struct impl_s : AlignedType<impl_s>
        {
            // Prioritize those member variables that require 16-byte alignment
            CBufferObject<CBChangesEveryDraw, 0>    cb_drawing_;        // Constant buffer bound to drawing objects
            CBufferObject<CBDrawStates, 1>          cb_states_;         // Constant buffer bound to drawing states
            CBufferObject<CBChangesEveryFrame, 2>   cb_frame_;          // Constant buffer bound to every frame, such as camera view matrix and camera location
            CBufferObject<CBChangesOnResize, 3>     cb_on_resize_;      // Constant buffer bound to window-resize event, such as camera projection
            CBufferObject<CBChangesRarely, 4>       cb_rarely_;         // Constant buffer that will never change

            int32_t is_dirty_;                                          // Mark whether any constant buffer object has changed
            std::vector<CBufferBase *> cb_objects_;                     // Manage all above constant buffer objects

            com_ptr<ID3D11VertexShader> vertex_shader_3d_;              // 3D vertex shader
            com_ptr<ID3D11PixelShader>  pixel_shader_3d_;               // 3D pixel shader
            com_ptr<ID3D11VertexShader> vertex_shader_2d_;              // 2D vertex shader
            com_ptr<ID3D11PixelShader>  pixel_shader_2d_;               // 2D pixel shader

            com_ptr<ID3D11InputLayout>  vertex_layout_3d_;              // 3D vertex input layout
            com_ptr<ID3D11InputLayout>  vertex_layout_2d_;              // 2D vertex input layout

            com_ptr<ID3D11ShaderResourceView> texture_;                 // Texture

            // Constructor
            impl_s() : is_dirty_(false) {}
            // Deconstruction
            ~impl_s() = default;
        };

        std::unique_ptr<impl_s> p_impl_;
    };

    // Use
    using BasicEffect = singleton_c<basic_effect_c>;
}







































