//
// Created by ZZK on 2023/5/31.
//

#include <Toy/Model/texture_manager.h>

#include <DDSTextureLoader/DDSTextureLoader11.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace toy::model
{
    struct StbImageDeleter
    {
        void operator()(uint8_t *pixel_data) const
        {
            stbi_image_free(pixel_data);
        }
    };

    TextureManager& TextureManager::get()
    {
        static TextureManager texture_manager{};
        return texture_manager;
    }

    void TextureManager::init(ID3D11Device *device)
    {
        m_device = device;
        m_device->GetImmediateContext(m_device_context.ReleaseAndGetAddressOf());

        // 1 X 1 empty texture
        com_ptr<ID3D11Texture2D> tex = nullptr;
        CD3D11_TEXTURE2D_DESC null_tex_desc{DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1};
        D3D11_SUBRESOURCE_DATA init_data{};
        auto white = static_cast<uint32_t>(-1);
        init_data.pSysMem = &white;
        init_data.SysMemPitch = 4;
        m_device->CreateTexture2D(&null_tex_desc, &init_data, tex.GetAddressOf());

        com_ptr<ID3D11ShaderResourceView> srv = nullptr;
        m_device->CreateShaderResourceView(tex.Get(), nullptr, srv.GetAddressOf());
        m_texture_srvs.try_emplace(0, srv.Get());
    }

    ID3D11ShaderResourceView *TextureManager::create_from_file(std::string_view filename, bool enable_mips, uint32_t force_SRGB)
    {
        XID file_id = string_to_id(filename);
        if (m_texture_srvs.count(file_id))
        {
            DX_CORE_WARN("{} texture asset has been loaded", filename);
            return m_texture_srvs[file_id].Get();
        }

        auto &&res = m_texture_srvs[file_id];
        std::wstring file_str = utf8_to_wstring(filename);
        auto hr = DirectX::CreateDDSTextureFromFileEx(
                m_device.Get(), (enable_mips ? m_device_context.Get() : nullptr),
                file_str.c_str(), 0, D3D11_USAGE_DEFAULT,
                D3D11_BIND_SHADER_RESOURCE, 0, 0,
                static_cast<DirectX::DDS_LOADER_FLAGS>(force_SRGB), nullptr, res.ReleaseAndGetAddressOf());

        if (FAILED(hr))
        {
            DX_CORE_INFO("Unsupported image type for DDS texture library, try to use stb image");
        } else
        {
            return res.Get();
        }

        com_ptr<ID3D11Texture2D> texture = nullptr;
        int32_t width = 0, height = 0, comp = 0;

        uint8_t* pixels = nullptr;
        auto is_hdr = stbi_is_hdr(filename.data());
        if (is_hdr)
        {
            auto float_pixels = stbi_loadf(filename.data(), &width, &height, &comp, STBI_rgb_alpha);
            pixels = reinterpret_cast<uint8_t *>(float_pixels);
            DX_CORE_INFO("Load HDR image: {}", filename);
        } else
        {
            pixels = stbi_load(filename.data(), &width, &height, &comp, STBI_rgb_alpha);
            DX_CORE_INFO("Load image: {}", filename);
        }

        std::unique_ptr<uint8_t, StbImageDeleter> img_data(pixels);
        if (img_data)
        {
            DXGI_FORMAT texture_format = is_hdr ? DXGI_FORMAT_R32G32B32A32_FLOAT : (force_SRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM);
            uint32_t row_pitch = is_hdr ? (static_cast<uint32_t>(STBI_rgb_alpha) * width * sizeof(float)) : (static_cast<uint32_t>(STBI_rgb_alpha) * width * sizeof(uint8_t));
            CD3D11_TEXTURE2D_DESC tex_desc(texture_format,
                                            width, height, 1,
                                            enable_mips ? 0 : 1,
                                            D3D11_BIND_SHADER_RESOURCE | (enable_mips ? D3D11_BIND_RENDER_TARGET : 0),
                                            D3D11_USAGE_DEFAULT, 0, 1, 0,
                                            enable_mips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0);

            m_device->CreateTexture2D(&tex_desc, nullptr, texture.GetAddressOf());
            // Upload texture data
            m_device_context->UpdateSubresource(texture.Get(), 0, nullptr, img_data.get(), row_pitch, 0);
            // Create SRV
            CD3D11_SHADER_RESOURCE_VIEW_DESC srv_desc(D3D11_SRV_DIMENSION_TEXTURE2D,texture_format);
            m_device->CreateShaderResourceView(texture.Get(), &srv_desc, res.ReleaseAndGetAddressOf());
            // Generate mipmap
            if (enable_mips)
            {
                m_device_context->GenerateMips(res.Get());
            }

            return res.Get();
        } else
        {
            DX_CORE_CRITICAL("Fail to create texture via stb image: {}", filename);
        }
    }

    ID3D11ShaderResourceView *TextureManager::create_from_memory(std::string_view name, void *data, size_t byte_width, bool enable_mips, uint32_t force_SRGB)
    {
        XID file_id = string_to_id(name);
        if (m_texture_srvs.count(file_id))
        {
            return m_texture_srvs[file_id].Get();
        }

        auto&& res = m_texture_srvs[file_id];
        int32_t width = 0, height = 0, comp = 0;
        std::unique_ptr<uint8_t, StbImageDeleter> img_data(stbi_load_from_memory(reinterpret_cast<uint8_t *>(data), static_cast<int32_t>(byte_width),
                                                                &width, &height, &comp, STBI_rgb_alpha));
        if (img_data)
        {
            CD3D11_TEXTURE2D_DESC tex_desc(force_SRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM,
                                        width, height, 1,
                                        enable_mips ? 0 : 1,
                                        D3D11_BIND_SHADER_RESOURCE | (enable_mips ? D3D11_BIND_RENDER_TARGET : 0),
                                        D3D11_USAGE_DEFAULT, 0, 1, 0,
                                        enable_mips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0);
            com_ptr<ID3D11Texture2D> tex = nullptr;
            m_device->CreateTexture2D(&tex_desc, nullptr, tex.GetAddressOf());
            // Update texture data
            m_device_context->UpdateSubresource(tex.Get(), 0, nullptr, img_data.get(), width * sizeof(uint32_t), 0);
            CD3D11_SHADER_RESOURCE_VIEW_DESC srv_desc(D3D11_SRV_DIMENSION_TEXTURE2D,
                                                    force_SRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM);
            // Create srv
            m_device->CreateShaderResourceView(tex.Get(), &srv_desc, res.ReleaseAndGetAddressOf());
            // Generate mipmap
            if (enable_mips)
            {
                m_device_context->GenerateMips(res.Get());
            }

            return res.Get();
        } else
        {
            DX_CORE_CRITICAL("Fail to create texture");
        }
    }

    bool TextureManager::add_texture(std::string_view name, ID3D11ShaderResourceView *texture)
    {
        XID name_id = string_to_id(name);
        return m_texture_srvs.try_emplace(name_id, texture).second;
    }

    void TextureManager::remove_texture(std::string_view name)
    {
        XID name_id = string_to_id(name);
        m_texture_srvs.erase(name_id);
    }

    void TextureManager::replace_with(std::string_view name, ID3D11ShaderResourceView *texture)
    {
        XID name_id = string_to_id(name);
        m_texture_srvs[name_id] = texture;
    }

    ID3D11ShaderResourceView* TextureManager::get_texture(std::string_view filename)
    {
        XID file_id = string_to_id(filename);
        if (m_texture_srvs.contains(file_id))
        {
            return m_texture_srvs[file_id].Get();
        }
        return nullptr;
    }

    ID3D11ShaderResourceView* TextureManager::get_null_texture()
    {
        return m_texture_srvs[0].Get();
    }
}


















