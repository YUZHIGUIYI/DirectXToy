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

        ID3D11Texture2D* get_texture() const { return m_texture.Get(); }
        ID3D11ShaderResourceView* get_shader_resource() const { return m_texture_srv.Get(); }

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

        ID3D11RenderTargetView* get_render_target() const { return m_texture_rtv.Get(); }
        ID3D11UnorderedAccessView* get_unordered_access() const { return m_texture_uav.Get(); }

        [[nodiscard]] uint32_t get_mip_levels() const { return m_mip_levels; }

        void set_unordered_access_view(ID3D11UnorderedAccessView* input_uav) { m_texture_uav = input_uav; }

        void set_debug_object_name(std::string_view name) override;

    private:
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

        ID3D11RenderTargetView* get_render_target() const { return m_texture_rtv.Get(); }

        uint32_t get_msaa_samples() const { return m_msaa_samples; }

        void set_debug_object_name(std::string_view name) override;

    private:
        uint32_t m_msaa_samples = 1;
        com_ptr<ID3D11RenderTargetView> m_texture_rtv = nullptr;
    };

    class TextureCube final : public Texture2DBase
    {
    public:
        TextureCube(ID3D11Device* device, uint32_t width, uint32_t height, DXGI_FORMAT format,
                    uint32_t mip_levels = 1,
                    uint32_t bind_flags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);
        ~TextureCube() override = default;

        [[nodiscard]] uint32_t get_mip_levels() const { return m_mip_levels; }

        ID3D11RenderTargetView* get_render_target() const { return m_texture_array_rtv.Get(); }
        ID3D11RenderTargetView* get_render_target(size_t array_idx) const { return m_render_target_elements[array_idx].Get(); }

        ID3D11UnorderedAccessView* get_unordered_access() const { return m_texture_array_uav.Get(); }
        ID3D11UnorderedAccessView* get_unordered_access(size_t array_idx) const { return m_unordered_access_elements[array_idx].Get(); }

        using Texture2DBase::get_shader_resource;

        ID3D11ShaderResourceView* get_shader_resource(size_t array_idx) const { return m_shader_resource_elements[array_idx].Get(); }

        void set_unordered_access_view(ID3D11UnorderedAccessView* input_uav) { m_texture_array_uav = input_uav; }

        void set_debug_object_name(std::string_view name) override;

    private:
        uint32_t m_mip_levels = 1;
        com_ptr<ID3D11RenderTargetView> m_texture_array_rtv = nullptr;
        com_ptr<ID3D11UnorderedAccessView> m_texture_array_uav = nullptr;
        std::vector<com_ptr<ID3D11RenderTargetView>> m_render_target_elements;
        std::vector<com_ptr<ID3D11UnorderedAccessView>> m_unordered_access_elements;
        std::vector<com_ptr<ID3D11ShaderResourceView>> m_shader_resource_elements;
    };

    class Texture2DArray final : public Texture2DBase
    {
    public:
        Texture2DArray(ID3D11Device* device, uint32_t width, uint32_t height, DXGI_FORMAT format,
                        uint32_t array_size, uint32_t mip_levels = 1,
                        uint32_t bind_flags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);
        ~Texture2DArray() override = default;

        [[nodiscard]] uint32_t get_mip_levels() const { return m_mip_levels; }
        [[nodiscard]] uint32_t get_array_size() const { return m_array_size; }

        ID3D11RenderTargetView* get_render_target() const { return m_texture_array_rtv.Get(); }
        ID3D11RenderTargetView* get_render_target(size_t array_idx) const { return m_render_target_elements[array_idx].Get(); }

        ID3D11UnorderedAccessView* get_unordered_access(size_t array_idx) const { return m_unordered_access_elements[array_idx].Get(); }

        using Texture2DBase::get_shader_resource;
        ID3D11ShaderResourceView* get_shader_resource(size_t array_idx) const { return m_shader_resource_elements[array_idx].Get(); }

        void set_debug_object_name(std::string_view name) override;

    private:
        uint32_t m_mip_levels = 1;
        uint32_t m_array_size = 1;
        com_ptr<ID3D11RenderTargetView> m_texture_array_rtv = nullptr;
        std::vector<com_ptr<ID3D11RenderTargetView>> m_render_target_elements;
        std::vector<com_ptr<ID3D11UnorderedAccessView>> m_unordered_access_elements;
        std::vector<com_ptr<ID3D11ShaderResourceView>> m_shader_resource_elements;
    };

    class Texture2DMSArray final : public Texture2DBase
    {
    public:
        Texture2DMSArray(ID3D11Device* device, uint32_t width, uint32_t height, DXGI_FORMAT format,
                            uint32_t array_size, const DXGI_SAMPLE_DESC& sample_desc,
                            uint32_t bind_flags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);

        ~Texture2DMSArray() override = default;

        [[nodiscard]] uint32_t get_array_size() const { return m_array_size; }
        [[nodiscard]] uint32_t get_msaa_samples() const { return m_msaa_samples; }

        ID3D11RenderTargetView* get_render_target() const { return m_texture_array_rtv.Get(); }
        ID3D11RenderTargetView* get_render_target(size_t array_idx) const { return m_render_target_elements[array_idx].Get(); }

        // Texture2DMSArray
        using Texture2DBase::get_shader_resource;
        // Texture2DMS
        ID3D11ShaderResourceView* get_shader_resource(size_t array_idx) const { return m_shader_resource_elements[array_idx].Get(); }

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

    class Depth2D final : public Texture2DBase
    {
    public:
        Depth2D(ID3D11Device* device, uint32_t width, uint32_t height,
                DepthStencilBitsFlag depth_stencil_bits_flag = DepthStencilBitsFlag::Depth_24Bits_Stencil_8Bits,
                uint32_t bind_flags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
        ~Depth2D() override = default;

        ID3D11DepthStencilView* get_depth_stencil() const { return m_texture_dsv.Get(); }

        void set_debug_object_name(std::string_view name) override;

    private:
        com_ptr<ID3D11DepthStencilView> m_texture_dsv;
    };

    class Depth2DMS final : public Texture2DBase
    {
    public:
        Depth2DMS(ID3D11Device* device, uint32_t width, uint32_t height,
                    const DXGI_SAMPLE_DESC& sampleDesc,
                    DepthStencilBitsFlag depthStencilBitsFlag = DepthStencilBitsFlag::Depth_24Bits_Stencil_8Bits,
                    uint32_t bindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
        ~Depth2DMS() override = default;

        [[nodiscard]] uint32_t get_msaa_samples() const { return m_msaa_samples; }
        ID3D11DepthStencilView* get_depth_stencil() const { return m_texture_dsv.Get(); }

        void set_debug_object_name(std::string_view name) override;

    private:
        uint32_t m_msaa_samples = 1;
        com_ptr<ID3D11DepthStencilView> m_texture_dsv;
    };

    class Depth2DArray final : public Texture2DBase
    {
    public:
        Depth2DArray(ID3D11Device* device, uint32_t width, uint32_t height, uint32_t array_size,
                        DepthStencilBitsFlag depth_stencil_bits_flag = DepthStencilBitsFlag::Depth_24Bits_Stencil_8Bits,
                        uint32_t bind_flags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
        ~Depth2DArray() override = default;

        uint32_t get_array_size() const { return m_array_size; }

        ID3D11DepthStencilView* get_depth_stencil() const { return m_depth_array_dsv.Get(); }
        ID3D11DepthStencilView* get_depth_stencil(size_t array_idx) const { return m_depth_stencil_elements[array_idx].Get(); }

        // Texture array
        using Texture2DBase::get_shader_resource;
        // Texture 2D
        ID3D11ShaderResourceView* get_shader_resource(size_t array_idx) const { return m_shader_resource_elements[array_idx].Get(); }

        // Set debug object name
        void set_debug_object_name(std::string_view name) override;

    private:
        uint32_t m_array_size = 1;
        com_ptr<ID3D11DepthStencilView> m_depth_array_dsv;
        std::vector<com_ptr<ID3D11DepthStencilView>> m_depth_stencil_elements;
        std::vector<com_ptr<ID3D11ShaderResourceView>> m_shader_resource_elements;
    };

    class Depth2DMSArray final : public Texture2DBase
    {
    public:
        Depth2DMSArray(ID3D11Device* device, uint32_t width, uint32_t height, uint32_t array_size,
                        const DXGI_SAMPLE_DESC& sample_desc,
                        DepthStencilBitsFlag depth_stencil_bits_flag = DepthStencilBitsFlag::Depth_24Bits_Stencil_8Bits,
                        uint32_t bind_flags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);

        ~Depth2DMSArray() override = default;

        uint32_t get_array_size() const { return m_array_size; }
        uint32_t get_msaa_samples() const { return m_msaa_samples; }

        ID3D11DepthStencilView* get_depth_stencil() const { return m_depth_array_dsv.Get(); }
        ID3D11DepthStencilView* get_depth_stencil(size_t array_idx) const { return m_depth_stencil_elements[array_idx].Get(); }

        // Texture 2D MS array
        using Texture2DBase::get_shader_resource;
        // Texture 2D MS
        ID3D11ShaderResourceView* get_shader_resource(size_t array_idx) const { return m_shader_resource_elements[array_idx].Get(); }

        // Set debug object name
        void set_debug_object_name(std::string_view name) override;

    private:
        uint32_t m_array_size = 1;
        uint32_t m_msaa_samples = 1;

        com_ptr<ID3D11DepthStencilView> m_depth_array_dsv;
        std::vector<com_ptr<ID3D11DepthStencilView>> m_depth_stencil_elements;
        std::vector<com_ptr<ID3D11ShaderResourceView>> m_shader_resource_elements;
    };

    template<typename T>
    concept texture_concept = std::is_same_v<TextureCube, T> || std::is_same_v<Texture2D, T>;

    template<typename T>
    requires texture_concept<T>
    static void create_texture_uav(ID3D11Device *device, T *texture, uint32_t mip_slice)
    {
        auto texture_base = texture->get_texture();
        if (!texture_base)
        {
            DX_CORE_WARN("Ensure that texture has been created");
            return;
        }

        D3D11_TEXTURE2D_DESC texture_desc{};
        texture_base->GetDesc(&texture_desc);
        if (!(texture_desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS))
        {
            DX_CORE_WARN("Ensure that texture's bind flags 'D3D11_BIND_UNORDERED_ACCESS' has been set");
            return;
        }

        D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
        uav_desc.Format = texture_desc.Format;
        if (texture_desc.ArraySize == 1)
        {
            uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
            uav_desc.Texture2D.MipSlice = mip_slice;
        } else
        {
            uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
            uav_desc.Texture2DArray.MipSlice = mip_slice;
            uav_desc.Texture2DArray.FirstArraySlice = 0;
            uav_desc.Texture2DArray.ArraySize = texture_desc.ArraySize;
        }

        ID3D11UnorderedAccessView* created_uav = nullptr;
        auto result = device->CreateUnorderedAccessView(texture_base, &uav_desc, &created_uav);
        if (FAILED(result))
        {
            DX_CORE_CRITICAL("Fail to create texture unordered access view");
        }
        texture->set_unordered_access_view(created_uav);
    }
}




