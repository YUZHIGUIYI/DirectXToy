//
// Created by ZZK on 2023/5/31.
//

#pragma once

#include <Toy/Core/d3d_util.h>

namespace toy::model
{
    class TextureManager
    {
    public:
        TextureManager() = default;
        ~TextureManager() = default;

        TextureManager(TextureManager&) = delete;
        TextureManager& operator=(const TextureManager&) = delete;
        TextureManager(TextureManager&&) = default;
        TextureManager& operator=(TextureManager&&) = default;

        void init(ID3D11Device* device);

        ID3D11ShaderResourceView* create_from_file(std::string_view filename, bool enable_mips = false, uint32_t force_SRGB = 0);
        ID3D11ShaderResourceView* create_from_memory(std::string_view name, void* data, size_t byte_width, bool enable_mips = false, uint32_t force_SRGB = 0);

        bool add_texture(std::string_view name, ID3D11ShaderResourceView* texture);
        void remove_texture(std::string_view name);

        ID3D11ShaderResourceView* get_texture(std::string_view filename);
        ID3D11ShaderResourceView* get_null_texture();

    private:
        com_ptr<ID3D11Device> m_device;
        com_ptr<ID3D11DeviceContext> m_device_context;
        std::unordered_map<XID, com_ptr<ID3D11ShaderResourceView>> m_texture_srvs;
    };

    using TextureManagerHandle = singleton_c<TextureManager>;
}




















