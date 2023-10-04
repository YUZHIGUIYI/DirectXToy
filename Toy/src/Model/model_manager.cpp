//
// Created by ZZK on 2023/5/31.
//

#include <Toy/Model/model_manager.h>
#include <Toy/Model/texture_manager.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace toy::model
{
    void Model::create_from_file(toy::model::Model &model, ID3D11Device *device, std::string_view file_name)
    {
        static_assert(sizeof(aiVector3D) == sizeof(DirectX::XMFLOAT3), "size of aiVector3D is not equal to sizeof DirectX::XMFLOAT3");

        model.materials.clear();
        model.meshes.clear();
        model.bounding_box = DirectX::BoundingBox{};

        Assimp::Importer importer;
        // Remove point and line primitive
        importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
        auto assimp_scene = importer.ReadFile(file_name.data(),
            aiProcess_ConvertToLeftHanded |     // Left hand coordinate
            aiProcess_CalcTangentSpace |               // Tangent space
            aiProcess_GenBoundingBoxes |               // Generate bounding box
            aiProcess_Triangulate |                    // Polygon splitting
            aiProcess_ImproveCacheLocality |           // Improve cache locality
            aiProcess_SortByPType);                    // Can remove no-triangle primitive

        if (!assimp_scene || (assimp_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !assimp_scene->HasMeshes())
        {
            DX_CORE_CRITICAL("Fail to load model asset, model file '{0}' may be incomplete", file_name);
        }

        using namespace DirectX;
        model.meshes.resize(assimp_scene->mNumMeshes);
        model.materials.resize(assimp_scene->mNumMaterials);
        for (uint32_t i = 0; i < assimp_scene->mNumMeshes; ++i)
        {
            auto&& mesh = model.meshes[i];
            auto ai_mesh = assimp_scene->mMeshes[i];
            uint32_t num_vertices = ai_mesh->mNumVertices;

            CD3D11_BUFFER_DESC buffer_desc{ 0, D3D11_BIND_VERTEX_BUFFER };
            D3D11_SUBRESOURCE_DATA init_data{ nullptr, 0, 0 };
            // Position
            if (ai_mesh->mNumVertices > 0)
            {
                init_data.pSysMem = ai_mesh->mVertices;
                buffer_desc.ByteWidth = num_vertices * sizeof(XMFLOAT3);
                device->CreateBuffer(&buffer_desc, &init_data, mesh.vertices.GetAddressOf());

                BoundingBox::CreateFromPoints(mesh.bounding_box, num_vertices, (const XMFLOAT3 *)ai_mesh->mVertices, sizeof(XMFLOAT3));
                if (i == 0)
                {
                    model.bounding_box = mesh.bounding_box;
                } else
                {
                    model.bounding_box.CreateMerged(model.bounding_box, model.bounding_box, mesh.bounding_box);
                }
            }
            // Normal
            if (ai_mesh->HasNormals())
            {
                init_data.pSysMem = ai_mesh->mNormals;
                buffer_desc.ByteWidth = num_vertices * sizeof(XMFLOAT3);
                device->CreateBuffer(&buffer_desc, &init_data, mesh.normals.GetAddressOf());
            }
            // Tangent and bitangent
            if (ai_mesh->HasTangentsAndBitangents())
            {
                std::vector<XMFLOAT4> tangents(num_vertices, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
                for (uint32_t index = 0; index < ai_mesh->mNumVertices; ++index)
                {
                    memcpy_s(&tangents[index], sizeof(XMFLOAT3), ai_mesh->mTangents + index, sizeof(XMFLOAT3));
                }

                init_data.pSysMem = tangents.data();
                buffer_desc.ByteWidth = ai_mesh->mNumVertices * sizeof(XMFLOAT4);
                device->CreateBuffer(&buffer_desc, &init_data, mesh.tangents.GetAddressOf());

                for (uint32_t index = 0; index < ai_mesh->mNumVertices; ++index)
                {
                    memcpy_s(&tangents[index], sizeof(XMFLOAT3), ai_mesh->mBitangents + index, sizeof(XMFLOAT3));
                }
                device->CreateBuffer(&buffer_desc, &init_data, mesh.bitangents.GetAddressOf());
            }
            // Texture coordinates
            uint32_t num_uvs = 8;
            while (num_uvs && !ai_mesh->HasTextureCoords(num_uvs - 1))
            {
                num_uvs--;
            }
            if (num_uvs > 0)
            {
                mesh.texcoord_arrays.resize(num_uvs);
                for (uint32_t row = 0; row < num_uvs; ++row)
                {
                    std::vector<XMFLOAT2> uvs(num_vertices);
                    for (uint32_t col = 0; col < num_vertices; ++col)
                    {
                        memcpy_s(&uvs[col], sizeof(XMFLOAT2), ai_mesh->mTextureCoords[row] + col, sizeof(XMFLOAT2));
                    }
                    init_data.pSysMem = uvs.data();
                    buffer_desc.ByteWidth = num_vertices * sizeof(XMFLOAT2);
                    device->CreateBuffer(&buffer_desc, &init_data, mesh.texcoord_arrays[row].GetAddressOf());
                }
            }
            // Index
            uint32_t num_faces = ai_mesh->mNumFaces;
            uint32_t num_indices = num_faces * 3;
            if (num_faces > 0)
            {
                mesh.index_count = num_indices;
                if (num_indices < 65536)
                {
                    std::vector<uint16_t> indices(num_indices);
                    for (size_t index = 0; index < num_faces; ++index)
                    {
                        indices[index * 3] = static_cast<uint16_t>(ai_mesh->mFaces[index].mIndices[0]);
                        indices[index * 3 + 1] = static_cast<uint16_t>(ai_mesh->mFaces[index].mIndices[1]);
                        indices[index * 3 + 2] = static_cast<uint16_t>(ai_mesh->mFaces[index].mIndices[2]);
                    }
                    buffer_desc = CD3D11_BUFFER_DESC{ static_cast<uint32_t>(num_indices * sizeof(uint16_t)), D3D11_BIND_INDEX_BUFFER };
                    init_data.pSysMem = indices.data();
                    device->CreateBuffer(&buffer_desc, &init_data, mesh.indices.GetAddressOf());
                } else
                {
                    std::vector<uint32_t> indices(num_indices);
                    for (size_t index = 0; index < num_faces; ++index)
                    {
                        memcpy_s(indices.data() + index * 3, sizeof(uint32_t) * 3, ai_mesh->mFaces[index].mIndices, sizeof(uint32_t) * 3);
                    }
                    buffer_desc = CD3D11_BUFFER_DESC{ static_cast<uint32_t>(num_indices * sizeof(uint32_t)), D3D11_BIND_INDEX_BUFFER };
                    init_data.pSysMem = indices.data();
                    device->CreateBuffer(&buffer_desc, &init_data, mesh.indices.GetAddressOf());
                }
            }
            // Material
            mesh.material_index = ai_mesh->mMaterialIndex;
        }

        for (uint32_t i = 0; i < assimp_scene->mNumMaterials; ++i)
        {
            auto&& material = model.materials[i];
            auto ai_material = assimp_scene->mMaterials[i];
            XMFLOAT4 vec{};
            float value{};
            uint32_t boolean{};
            uint32_t num = 3;

            if (aiReturn_SUCCESS == ai_material->Get(AI_MATKEY_COLOR_AMBIENT, (float*)&vec, &num))
            {
                material.set("$AmbientColor", vec);
            }
            if (aiReturn_SUCCESS == ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, (float*)&vec, &num))
            {
                material.set("$DiffuseColor", vec);
            }
            if (aiReturn_SUCCESS == ai_material->Get(AI_MATKEY_COLOR_SPECULAR, (float*)&vec, &num))
            {
                material.set("$SpecularColor", vec);
            }
            if (aiReturn_SUCCESS == ai_material->Get(AI_MATKEY_SPECULAR_FACTOR, value))
            {
                material.set("$SpecularFactor", value);
            }
            if (aiReturn_SUCCESS == ai_material->Get(AI_MATKEY_COLOR_EMISSIVE, (float*)&vec, &num))
            {
                material.set("$EmissiveColor", vec);
            }
            if (aiReturn_SUCCESS == ai_material->Get(AI_MATKEY_OPACITY, value))
            {
                material.set("$Opacity", value);
            }
            if (aiReturn_SUCCESS == ai_material->Get(AI_MATKEY_COLOR_TRANSPARENT, (float*)&vec, &num))
            {
                material.set("$TransparentColor", vec);
            }
            if (aiReturn_SUCCESS == ai_material->Get(AI_MATKEY_COLOR_REFLECTIVE, (float*)&vec, &num))
            {
                material.set("$ReflectiveColor", vec);
            }

            aiString ai_path{};
            std::filesystem::path tex_file_name{};
            std::string tex_name{};

            // Texture manage
            auto try_create_texture = [&file_name, &assimp_scene, &ai_material, &material, &ai_path, &tex_file_name, &tex_name]
                            (aiTextureType type, std::string_view property_name, bool gen_mips = false, uint32_t force_SRGB = 0)
            {
                if (!ai_material->GetTextureCount(type))
                {
                    return;
                }

                ai_material->GetTexture(type, 0, &ai_path);

                // If texture has been loaded
                if (ai_path.data[0] == '*')
                {
                    tex_name = file_name;
                    tex_name += ai_path.C_Str();
                    char* end_str = nullptr;
                    aiTexture* p_tex = assimp_scene->mTextures[std::strtol(ai_path.data + 1, &end_str, 10)];
                    TextureManager::get().create_from_memory(tex_name, p_tex->pcData, (p_tex->mHeight ? p_tex->mWidth * p_tex->mHeight : p_tex->mWidth),
                                                                    gen_mips, force_SRGB);
                    material.set(property_name, std::string(tex_name));
                }
                // Texture indexed by file name
                else
                {
                    tex_file_name = file_name;
                    tex_file_name = tex_file_name.parent_path() / ai_path.C_Str();
                    TextureManager::get().create_from_file(tex_file_name.string(), gen_mips, force_SRGB);
                    material.set(property_name, tex_file_name.string());
                }
            };

            // Create textures
            try_create_texture(aiTextureType_DIFFUSE, material_semantics_name(MaterialSemantics::DiffuseMap), true, 1);
            try_create_texture(aiTextureType_SPECULAR, material_semantics_name(MaterialSemantics::SpecularMap), true, 1);
            try_create_texture(aiTextureType_NORMALS, material_semantics_name(MaterialSemantics::NormalMap));
            try_create_texture(aiTextureType_BASE_COLOR, material_semantics_name(MaterialSemantics::AlbedoMap), true, 1);
            try_create_texture(aiTextureType_NORMAL_CAMERA, material_semantics_name(MaterialSemantics::NormalCameraMap));
            try_create_texture(aiTextureType_METALNESS, material_semantics_name(MaterialSemantics::MetalnessMap));
            try_create_texture(aiTextureType_DIFFUSE_ROUGHNESS, material_semantics_name(MaterialSemantics::RoughnessMap));
            try_create_texture(aiTextureType_AMBIENT_OCCLUSION, material_semantics_name(MaterialSemantics::AmbientOcclusionMap));

            // Set diffuse color and opacity and metalness and roughness material properties
            if (auto diffuse_color_name = material_semantics_name(MaterialSemantics::DiffuseColor); !material.has_property(diffuse_color_name))
            {
                material.set<XMFLOAT4>(diffuse_color_name, XMFLOAT4{ 0.8f, 0.8f, 0.8f, 1.0f });
            }
            if (auto opacity_name = material_semantics_name(MaterialSemantics::Opacity); !material.has_property(opacity_name))
            {
                material.set<float>(opacity_name, 1.0f);
            }
            if (auto metalness_name = material_semantics_name(MaterialSemantics::Metalness); !material.has_property(metalness_name))
            {
                material.set<float>(metalness_name, 0.5f);
            }
            if (auto roughness_name = material_semantics_name(MaterialSemantics::Roughness); !material.has_property(roughness_name))
            {
                material.set<float>(roughness_name, 0.5f);
            }
        }
    }

    void Model::create_from_geometry(toy::model::Model &model, ID3D11Device *device, const geometry::GeometryData &data,
                                        bool is_dynamic)
    {
        using namespace DirectX;
        // Default material
        model.materials.resize(1);
        model.materials[0].set<XMFLOAT4>(material_semantics_name(MaterialSemantics::AmbientColor), XMFLOAT4{0.2f, 0.2f, 0.2f, 1.0f });
        model.materials[0].set<XMFLOAT4>(material_semantics_name(MaterialSemantics::DiffuseColor), XMFLOAT4{0.8f, 0.8f, 0.8f, 1.0f });
        model.materials[0].set<XMFLOAT4>(material_semantics_name(MaterialSemantics::SpecularColor), XMFLOAT4{0.2f, 0.2f, 0.2f, 1.0f });
        model.materials[0].set<float>(material_semantics_name(MaterialSemantics::SpecularFactor), 10.0f);
        model.materials[0].set<float>(material_semantics_name(MaterialSemantics::Opacity), 1.0f);
        model.materials[0].set<float>(material_semantics_name(MaterialSemantics::Metalness), 0.5f);
        model.materials[0].set<float>(material_semantics_name(MaterialSemantics::Roughness), 0.5f);

        model.meshes.resize(1);
        model.meshes[0].texcoord_arrays.resize(1);
        model.meshes[0].vertex_count = (uint32_t)data.vertices.size();
        model.meshes[0].index_count = (uint32_t)(!data.indices16.empty() ? data.indices16.size() : data.indices32.size());
        model.meshes[0].material_index = 0;

        CD3D11_BUFFER_DESC buffer_desc(0,
                                        D3D11_BIND_VERTEX_BUFFER,
                                        is_dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT,
                                        is_dynamic ? D3D11_CPU_ACCESS_WRITE : 0);
        D3D11_SUBRESOURCE_DATA init_data{ nullptr, 0, 0 };

        init_data.pSysMem = data.vertices.data();
        buffer_desc.ByteWidth = (uint32_t)(data.vertices.size() * sizeof(XMFLOAT3));
        device->CreateBuffer(&buffer_desc, &init_data, model.meshes[0].vertices.GetAddressOf());

        if (!data.normals.empty())
        {
            init_data.pSysMem = data.normals.data();
            buffer_desc.ByteWidth = (uint32_t)data.normals.size() * sizeof(XMFLOAT3);
            device->CreateBuffer(&buffer_desc, &init_data, model.meshes[0].normals.GetAddressOf());
        }

        if (!data.texcoords.empty())
        {
            init_data.pSysMem = data.texcoords.data();
            buffer_desc.ByteWidth = (uint32_t)data.texcoords.size() * sizeof(XMFLOAT2);
            device->CreateBuffer(&buffer_desc, &init_data, model.meshes[0].texcoord_arrays[0].GetAddressOf());
        }

        if (!data.tangents.empty())
        {
            init_data.pSysMem = data.tangents.data();
            buffer_desc.ByteWidth = (uint32_t)data.tangents.size() * sizeof(XMFLOAT4);
            device->CreateBuffer(&buffer_desc, &init_data, model.meshes[0].tangents.GetAddressOf());
        }

        if (!data.indices16.empty())
        {
            init_data.pSysMem = data.indices16.data();
            buffer_desc = CD3D11_BUFFER_DESC((uint16_t)data.indices16.size() * sizeof(uint16_t), D3D11_BIND_INDEX_BUFFER);
            buffer_desc.Usage = D3D11_USAGE_DEFAULT;
            buffer_desc.CPUAccessFlags = 0;
            device->CreateBuffer(&buffer_desc, &init_data, model.meshes[0].indices.GetAddressOf());
        }
        else
        {
            init_data.pSysMem = data.indices32.data();
            buffer_desc = CD3D11_BUFFER_DESC((uint32_t)data.indices32.size() * sizeof(uint32_t), D3D11_BIND_INDEX_BUFFER);
            buffer_desc.Usage = D3D11_USAGE_DEFAULT;
            buffer_desc.CPUAccessFlags = 0;
            device->CreateBuffer(&buffer_desc, &init_data, model.meshes[0].indices.GetAddressOf());
        }
    }

    void Model::set_debug_object_name(std::string_view name)
    {
        // TODO
    }

    ModelManager& ModelManager::get()
    {
        static ModelManager model_manager{};
        return model_manager;
    }

    void ModelManager::init(ID3D11Device *device)
    {
        m_device_ = device;
        m_device_->GetImmediateContext(m_device_context_.ReleaseAndGetAddressOf());
    }

    Model* ModelManager::create_from_file(std::string_view file_name)
    {
        return create_from_file(file_name, file_name);
    }

    Model* ModelManager::create_from_file(std::string_view name, std::string_view file_name)
    {
        XID model_id = string_to_id(name);
        auto& model = m_models[model_id];
        Model::create_from_file(model, m_device_.Get(), file_name);
        return &model;
    }

    Model* ModelManager::create_from_geometry(std::string_view name, const geometry::GeometryData &data,
                                                bool is_dynamic)
    {
        XID model_id = string_to_id(name);
        auto& model = m_models[model_id];
        Model::create_from_geometry(model, m_device_.Get(), data, is_dynamic);
        return &model;
    }

    const Model* ModelManager::get_model(std::string_view name) const
    {
        XID name_id = string_to_id(name);
        if (auto it = m_models.find(name_id); it != m_models.end())
        {
            return &(it->second);
        }
        return nullptr;
    }

    Model* ModelManager::get_model(std::string_view name)
    {
        XID name_id = string_to_id(name);
        if (m_models.count(name_id))
        {
            return &m_models[name_id];
        }
        return nullptr;
    }
}






























