//
// Created by ZZK on 2023/6/1.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{

    class Texture2DBase
    {
    public:
        Texture2DBase(ID3D11Device* device, const CD3D11_TEXTURE2D_DESC& tex_desc, const CD3D11_SHADER_RESOURCE_VIEW_DESC& srv_desc);
        virtual ~Texture2DBase() = default;

        ID3D11Texture2D* get_texture() { return m_texture.Get(); }
        ID3D11ShaderResourceView* get_shader_resource() { return m_texture_srv.Get(); }

        [[nodiscard]] uint32_t get_width() const { return m_width; }
        [[nodiscard]] uint32_t get_height() const { return m_height; }

        // Set debug object name
        virtual void set_debug_object_name(std::string_view name);

    protected:
        com_ptr<ID3D11Texture2D> m_texture = nullptr;
        com_ptr<ID3D11ShaderResourceView> m_texture_srv = nullptr;
        uint32_t m_width{};
        uint32_t m_height{};
    };

    class Texture2D : public Texture2DBase
    {
    public:
        Texture2D(ID3D11Device* device, uint32_t width, uint32_t height, DXGI_FORMAT format,
                    uint32_t mip_levels = 1,
                    uint32_t bind_flags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);

        ~Texture2D() override = default;

        Texture2D(const Texture2D&) = delete;
        Texture2D& operator=(const Texture2D&) = delete;
        Texture2D(Texture2D&&) = default;
        Texture2D& operator=(Texture2D&&) = default;

        ID3D11RenderTargetView* get_render_target() { return m_texture_rtv.Get(); }
        ID3D11UnorderedAccessView* get_unordered_access() { return m_texture_uav.Get(); }

        [[nodiscard]] uint32_t get_mip_levels() const { return m_mip_levels; }

        void set_debug_object_name(std::string_view name) override;

    protected:
        uint32_t m_mip_levels = 1;
        com_ptr<ID3D11RenderTargetView> m_texture_rtv = nullptr;
        com_ptr<ID3D11UnorderedAccessView> m_texture_uav = nullptr;
    };

    class Texture2DMS : public Texture2DBase
    {
    public:
        Texture2DMS(ID3D11Device* device, uint32_t width, uint32_t height, DXGI_FORMAT format,
                    const DXGI_SAMPLE_DESC& sample_desc,
                    uint32_t bind_flags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
        ~Texture2DMS() override = default;

        ID3D11RenderTargetView* get_render_target() { return m_texture_rtv.Get(); }

        uint32_t get_msaa_samples() const { return m_msaa_samples; }

        void set_debug_object_name(std::string_view name) override;

    private:
        uint32_t m_msaa_samples = 1;
        com_ptr<ID3D11RenderTargetView> m_texture_rtv = nullptr;
    };

    class TextureCube : public Texture2DBase
    {
    public:
        TextureCube(ID3D11Device* device, uint32_t width, uint32_t height, DXGI_FORMAT format,
                    uint32_t mip_levels = 1,
                    uint32_t bind_flags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);
        ~TextureCube() override = default;

        [[nodiscard]] uint32_t get_mip_levels() const { return m_mip_levels; }

        ID3D11RenderTargetView* get_render_target() { return m_texture_array_rtv.Get(); }
        ID3D11RenderTargetView* get_render_target(size_t array_idx) { return m_render_target_elements[array_idx].Get(); }

        ID3D11UnorderedAccessView* get_unordered_access(size_t array_idx) { return m_unordered_access_elements[array_idx].Get(); }

        using Texture2DBase::get_shader_resource;

        ID3D11ShaderResourceView* get_shader_resource(size_t array_idx) { return m_shader_resource_elements[array_idx].Get(); }

        void set_debug_object_name(std::string_view name) override;

    private:
        uint32_t m_mip_levels = 1;
        com_ptr<ID3D11RenderTargetView> m_texture_array_rtv = nullptr;
        std::vector<com_ptr<ID3D11RenderTargetView>> m_render_target_elements;
        std::vector<com_ptr<ID3D11UnorderedAccessView>> m_unordered_access_elements;
        std::vector<com_ptr<ID3D11ShaderResourceView>> m_shader_resource_elements;
    };

    class Texture2DArray : public Texture2DBase
    {
    public:
        Texture2DArray(ID3D11Device* device, uint32_t width, uint32_t height, DXGI_FORMAT format,
                        uint32_t array_size, uint32_t mip_levels = 1,
                        uint32_t bind_flags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);
        ~Texture2DArray() override = default;

        [[nodiscard]] uint32_t get_mip_levels() const { return m_mip_levels; }
        [[nodiscard]] uint32_t get_array_size() const { return m_array_size; }

        ID3D11RenderTargetView* get_render_target() { return m_texture_array_rtv.Get(); }
        ID3D11RenderTargetView* get_render_target(size_t array_idx) { return m_render_target_elements[array_idx].Get(); }

        ID3D11UnorderedAccessView* get_unordered_access(size_t array_idx) { return m_unordered_access_elements[array_idx].Get(); }

        using Texture2DBase::get_shader_resource;
        ID3D11ShaderResourceView* get_shader_resource(size_t array_idx) { return m_shader_resource_elements[array_idx].Get(); }

        void set_debug_object_name(std::string_view name) override;

    private:
        uint32_t m_mip_levels = 1;
        uint32_t m_array_size = 1;
        com_ptr<ID3D11RenderTargetView> m_texture_array_rtv = nullptr;
        std::vector<com_ptr<ID3D11RenderTargetView>> m_render_target_elements;
        std::vector<com_ptr<ID3D11UnorderedAccessView>> m_unordered_access_elements;
        std::vector<com_ptr<ID3D11ShaderResourceView>> m_shader_resource_elements;
    };

    class Texture2DMSArray : public Texture2DBase
    {
    public:
        Texture2DMSArray(ID3D11Device* device, uint32_t width, uint32_t height, DXGI_FORMAT format,
                            uint32_t array_size, const DXGI_SAMPLE_DESC& sample_desc,
                            uint32_t bind_flags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);

        ~Texture2DMSArray() override = default;

        [[nodiscard]] uint32_t get_array_size() const { return m_array_size; }
        [[nodiscard]] uint32_t get_msaa_samples() const { return m_msaa_samples; }

        ID3D11RenderTargetView* get_render_target() { return m_texture_array_rtv.Get(); }
        ID3D11RenderTargetView* get_render_target(size_t array_idx) { return m_render_target_elements[array_idx].Get(); }

        // Texture2DMSArray
        using Texture2DBase::get_shader_resource;
        // Texture2DMS
        ID3D11ShaderResourceView* get_shader_resource(size_t array_idx) { return m_shader_resource_elements[array_idx].Get(); }

        // Set debug object name
        void set_debug_object_name(std::string_view name) override;

    private:
        uint32_t m_msaa_samples = 1;
        uint32_t m_array_size = 1;

        com_ptr<ID3D11RenderTargetView> m_texture_array_rtv;    // Point to texture array
        std::vector<com_ptr<ID3D11RenderTargetView>> m_render_target_elements;
        std::vector<com_ptr<ID3D11ShaderResourceView>> m_shader_resource_elements;
    };

    enum class DepthStencilBitsFlag
    {
        Depth_16Bits = 0,
        Depth_24Bits_Stencil_8Bits = 1,
        Depth_32Bits = 2,
        Depth_32Bits_Stencil_8Bits_Unused_24Bits = 3,
    };

    class Depth2D : public Texture2DBase
    {
    public:
        Depth2D(ID3D11Device* device, uint32_t width, uint32_t height,
                DepthStencilBitsFlag depth_stencil_bits_flag = DepthStencilBitsFlag::Depth_24Bits_Stencil_8Bits,
                uint32_t bind_flags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
        ~Depth2D() override = default;

        ID3D11DepthStencilView* get_depth_stencil() { return m_texture_dsv.Get(); }

        void set_debug_object_name(std::string_view name) override;

    private:
        com_ptr<ID3D11DepthStencilView> m_texture_dsv;
    };

    class Depth2DArray : public Texture2DBase
    {
    public:
        Depth2DArray(ID3D11Device* device, uint32_t width, uint32_t height, uint32_t array_size,
                        DepthStencilBitsFlag depth_stencil_bits_flag = DepthStencilBitsFlag::Depth_24Bits_Stencil_8Bits,
                        uint32_t bind_flags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
        ~Depth2DArray() override = default;

        uint32_t get_array_size() const { return m_array_size; }

        ID3D11DepthStencilView* get_depth_stencil() { return m_depth_array_dsv.Get(); }
        ID3D11DepthStencilView* get_depth_stencil(size_t array_idx) { return m_depth_stencil_elements[array_idx].Get(); }

        // Texture array
        using Texture2DBase::get_shader_resource;
        // Texture 2D
        ID3D11ShaderResourceView* get_shader_resource(size_t array_idx) { return m_shader_resource_elements[array_idx].Get(); }

        // Set debug object name
        void set_debug_object_name(std::string_view name) override;

    private:
        uint32_t m_array_size = 1;
        com_ptr<ID3D11DepthStencilView> m_depth_array_dsv;
        std::vector<com_ptr<ID3D11DepthStencilView>> m_depth_stencil_elements;
        std::vector<com_ptr<ID3D11ShaderResourceView>> m_shader_resource_elements;
    };

    class Depth2DMSArray : public Texture2DBase
    {
    public:
        Depth2DMSArray(ID3D11Device* device, uint32_t width, uint32_t height, uint32_t array_size,
                        const DXGI_SAMPLE_DESC& sample_desc,
                        DepthStencilBitsFlag depth_stencil_bits_flag = DepthStencilBitsFlag::Depth_24Bits_Stencil_8Bits,
                        uint32_t bind_flags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);

        ~Depth2DMSArray() override = default;

        uint32_t get_array_size() const { return m_array_size; }
        uint32_t get_msaa_samples() const { return m_msaa_samples; }

        ID3D11DepthStencilView* get_depth_stencil() { return m_depth_array_dsv.Get(); }
        ID3D11DepthStencilView* get_depth_stencil(size_t array_idx) { return m_depth_stencil_elements[array_idx].Get(); }

        // Texture 2D MS array
        using Texture2DBase::get_shader_resource;
        // Texture 2D MS
        ID3D11ShaderResourceView* get_shader_resource(size_t array_idx) { return m_shader_resource_elements[array_idx].Get(); }

        // Set debug object name
        void set_debug_object_name(std::string_view name) override;

    private:
        uint32_t m_array_size = 1;
        uint32_t m_msaa_samples = 1;

        com_ptr<ID3D11DepthStencilView> m_depth_array_dsv;
        std::vector<com_ptr<ID3D11DepthStencilView>> m_depth_stencil_elements;
        std::vector<com_ptr<ID3D11ShaderResourceView>> m_shader_resource_elements;
    };
}




