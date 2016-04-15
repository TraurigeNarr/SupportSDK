#include "stdafx.h"

#include "MeshSystem.h"

#include "Core.h"
#include "Render/Commands.h"
#include "Render/HardwareBufferManagerBase.h"
#include "Render/IRenderer.h"
#include "Applications/ApplicationBase.h"
#include "Render/RenderWorld.h"
#include "Render/MaterialManager.h"
#include "Resources/ResourceManager.h"

#include "Systems/TransformationsSystem.h"
#include "EntityManager.h"
#include "FileSystem/FileSystem.h"

#include "PropertyReaders.h"

#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace SDK
{
	struct MeshInformation
	{
		typedef Render::Mesh ResultType;

		Render::BufferUsageFormat m_ver_usage;
		Render::BufferUsageFormat m_ind_usage;
		std::string m_description_path;
	};

	namespace Resources
	{	
		namespace Serialization
		{
			
			template <>
			struct LoaderImpl < Render::Mesh >
			{
				static void ProcessMaterials(Render::Mesh& io_mesh, const PropertyElement& i_model_description)
				{
					using MeshMaterial = std::pair<size_t, std::string>;
					std::vector<MeshMaterial> materials;
					const auto end = i_model_description.end<PropertyElement>();
					for (auto it = i_model_description.begin<PropertyElement>(); it != end; ++it)
					{
						const std::string submesh_name = it->GetValue<std::string>("name");
						const std::string material_name = it->GetValue<std::string>("material");
						if (submesh_name.empty() || material_name.empty())
						{
							assert(false && "[Mesh] Empty name");
							continue;
						}
						materials.emplace_back(Utilities::hash_function(submesh_name), material_name);
					}
					auto p_load_manager = Core::GetGlobalObject<Resources::ResourceManager>();
					for (size_t i = 0; i < io_mesh.GetSubmeshNumber(); ++i)
					{
						Render::Mesh::SubMesh& sub_mesh = io_mesh.GetSubmesh(i);
						const size_t submesh_name = Utilities::hash_function(sub_mesh.m_name);
						auto it = std::find_if(materials.begin(), materials.end(), [submesh_name](const MeshMaterial& mat)
						{
							return mat.first == submesh_name;
						});
						// not found
						if (it == materials.end())
							continue;
						// mat name it->second
						Render::MaterialHandle material_handle = p_load_manager->GetHandleToResource<Render::Material>(it->second);
						if (material_handle == Render::MaterialHandle::InvalidHandle())
							continue;
						sub_mesh.m_materials.push_back(material_handle);
					}
				}

				static std::pair<LoadResult, Render::Mesh> ProcessMesh(const aiScene* ip_scene, MeshInformation i_info)
				{
					Render::Mesh result_mesh;
					struct Vertex
					{
						// Position
						Vector3 Position;
						// Normal
						Vector3 Normal;
						// TexCoords
						Vector2 TexCoords;
					};
					std::vector<Vertex> vertices;
					std::vector<int> indices;
					auto p_mgr = Core::GetRenderer()->GetHardwareBufferMgr();
					volatile int vertex_count = 0;
					volatile int trig_count = 0;
					for (uint im = 0; im < ip_scene->mNumMeshes; ++im)
					{
						const aiMesh* p_mesh = ip_scene->mMeshes[im];
						if (!p_mesh->HasPositions())
						{
							assert(false && "Without positions");
							continue;
						}
						vertices.clear();
						indices.clear();
						// Walk through each of the mesh's vertices
						for (uint i = 0; i < p_mesh->mNumVertices; i++)
						{
							Vertex vertex;
							Vector3 vector; // We declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
							// Positions
							vector[0] = p_mesh->mVertices[i].x;
							vector[1] = p_mesh->mVertices[i].y;
							vector[2] = p_mesh->mVertices[i].z;
							vertex.Position = vector;
							// Normals
							if (p_mesh->HasFaces())
							{
								vector[0] = p_mesh->mNormals[i].x;
								vector[1] = p_mesh->mNormals[i].y;
								vector[2] = p_mesh->mNormals[i].z;
								vertex.Normal = vector;
							}
							// Texture Coordinates
							if (p_mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
							{
								Vector2 vec;
								// A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
								// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
								vec[0] = p_mesh->mTextureCoords[0][i].x;
								vec[1] = p_mesh->mTextureCoords[0][i].y;
								vertex.TexCoords = vec;
							}
							else
								vertex.TexCoords = Vector2{ 0.0f, 0.0f };
							vertices.push_back(vertex);
						}
						// Now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
						for (uint i = 0; i < p_mesh->mNumFaces; i++)
						{
							aiFace face = p_mesh->mFaces[i];
							// Retrieve all indices of the face and store them in the indices vector
							for (uint j = 0; j < face.mNumIndices; j++)
								indices.push_back(face.mIndices[j]);
						}
						
						auto ver_buf = p_mgr->CreateHardwareBuffer(vertices.size()*sizeof(Vertex), i_info.m_ver_usage, &vertices[0]);
						Render::HardwareIndexBuffer::IndexType ind_type = Render::HardwareIndexBuffer::IndexType::Int;
						/*
						TODO: correct determination of vertex types
						if (indices.size() < 65535)
						ind_type = HardwareIndexBuffer::IndexType::Short;
						else if(indices.size() < 255)
						ind_type = HardwareIndexBuffer::IndexType::Byte;*/
						auto ind_buf = p_mgr->CreateIndexBuffer(ind_type, indices.size(), i_info.m_ind_usage, Render::PrimitiveType::Triangles, &indices[0]);
						auto pos_layout = p_mgr->CreateLayout(ver_buf, 3, Render::VertexSemantic::Position, Render::ComponentType::Float, false, sizeof(Vertex), 0);
						auto normals_layout = p_mgr->CreateLayout(ver_buf, 3, Render::VertexSemantic::Normal, Render::ComponentType::Float, false, sizeof(Vertex), offsetof(Vertex, Normal));
						auto uv_layout = p_mgr->CreateLayout(ver_buf, 2, Render::VertexSemantic::TextureCoordinates, Render::ComponentType::Float, false, sizeof(Vertex), offsetof(Vertex, TexCoords));
						vertex_count += vertices.size();
						trig_count += indices.size() / 3;
						result_mesh.AddSubmesh(p_mesh->mName.C_Str(), ver_buf, pos_layout, normals_layout, uv_layout, ind_buf);
					}
					
					if (!i_info.m_description_path.empty())
					{
						PropretyReader<(int)ReaderType::SDKFormat> reader;
						PropertyElement root = reader.Parse(i_info.m_description_path);
						ProcessMaterials(result_mesh, root);
					}
					
					return std::make_pair(LoadResult::Success, result_mesh);
				}

				static std::pair<LoadResult, Render::Mesh> Load(std::istream* ip_streams[1], MeshInformation i_info)
				{
					const std::string file_data = FS::ReadFileToString(*ip_streams[0]);
					if (file_data.empty())
					{
						assert(false && "File data is empty");
						return std::make_pair(LoadResult::Failure, Render::Mesh());
					}
					// Read file via ASSIMP
					Assimp::Importer importer;
					const aiScene* scene = importer.ReadFileFromMemory(file_data.c_str(), file_data.size(), aiProcess_Triangulate | aiProcess_FlipUVs);
					// Check for errors
					if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
					{
						std::string message = importer.GetErrorString();
						assert(false && "ASSIMP ERROR");
						return std::make_pair(LoadResult::Failure, Render::Mesh());
					}

					return ProcessMesh(scene, i_info);
				}

				// TODO: need some limitations on size of loaded meshes?
				static InternalHandle CreateNewHandle()
				{
					auto& handlers = Render::g_mesh_system.m_handlers;

					InternalHandle new_index{ -1, -1 };
					for (size_t i = 0; i < handlers.size(); ++i)
					{
						if (handlers[i].index == -1)
						{
							new_index.index = i;
							new_index.generation = handlers[i].generation;
							handlers[i].index = i;
							break;
						}
					}

					if (new_index.index == -1)
					{
						// TODO: resize vector normally - with some strategy - not push_back
						handlers.push_back(Render::MeshHandle());
						const int index = handlers.size() - 1;						
						handlers[index].generation = 0;
						handlers[index].index = index;

						new_index.index = index;
						new_index.generation = 0;
					}

					return new_index;
				}

				static void RemoveHandle(InternalHandle i_handle)
				{
					auto& handlers = Render::g_mesh_system.m_handlers;
					assert(i_handle.index < static_cast<int>(handlers.size()));
					++handlers[i_handle.index].generation;
					handlers[i_handle.index].index = -1;
				}

				static void UnloadResource(InternalHandle i_handle)
				{
					auto& handlers = Render::g_mesh_system.m_handlers;
					assert(i_handle.index < static_cast<int>(handlers.size()));
					++handlers[i_handle.index].generation;
					handlers[i_handle.index].index = -1;

					auto& meshes = Render::g_mesh_system.m_meshes;
					assert(i_handle.index < static_cast<int>(meshes.size()));
					// release resources for mesh
					auto p_mgr = Core::GetRenderer()->GetHardwareBufferMgr();
					Render::Mesh& mesh = meshes[i_handle.index];
					for (size_t i = 0; i < mesh.GetSubmeshNumber(); ++i)
					{
						const Render::Mesh::SubMesh& sub_mesh = mesh.GetSubmesh(i);
						p_mgr->DestroyBuffer(sub_mesh.m_vertex_buffer);
						p_mgr->DestroyLayout(sub_mesh.m_pos_layout);
						p_mgr->DestroyLayout(sub_mesh.m_normal_layout);
						p_mgr->DestroyLayout(sub_mesh.m_uv_layout);
						p_mgr->DestroyBuffer(sub_mesh.m_index_buffer);
					}					
				}

				static void Register(InternalHandle i_handle, Render::Mesh i_mesh)
				{					
					auto& meshes = Render::g_mesh_system.m_meshes;					
					if (static_cast<int>(meshes.size()) < i_handle.index)
					{
						// TODO: increase buffer strategy
						const size_t RANGE = 1000;
						float coeff = 2;
						if (meshes.size() > RANGE)
							coeff = 1.5f;
						meshes.resize(static_cast<size_t>(meshes.size() * coeff));
					}
					

					meshes[i_handle.index] = i_mesh;
				}
			};
		
		} // Serialization

	} // Resources

	namespace Render
	{
		static MeshSystem mesh_system;
		MeshSystem& g_mesh_system = mesh_system;

		MeshSystem::MeshSystem()
		{
			// TODO: choose initial size - provide generic strategy for allocating such types of buffers
			const size_t INITIAL_SIZE = 10;
			m_meshes.resize(INITIAL_SIZE);
			m_handlers.resize(INITIAL_SIZE);
			for (auto& handler : m_handlers)
				handler.index = -1;
		}

		MeshSystem::~MeshSystem()
		{

		}

		void MeshSystem::Initialize()
		{
			auto p_load_manager = Core::GetGlobalObject<Resources::ResourceManager>();
			p_load_manager->RegisterLoader<MeshSystem, Mesh>(*this, &MeshSystem::LoadMesh, "mesh");
			p_load_manager->RegisterLoader<MeshSystem, Mesh>(*this, &MeshSystem::LoadModel, "model");
		}

		void MeshSystem::Release()
		{
			auto p_load_manager = Core::GetGlobalObject<Resources::ResourceManager>();
			p_load_manager->Unregister<MeshSystem, Mesh>("mesh");
			p_load_manager->Unregister<MeshSystem, Mesh>("model");
		}

		BufferUsageFormat StringToFormat(const std::string& i_format)
		{
			if (i_format == "static")
				return BufferUsageFormat::Static;
			return BufferUsageFormat::Static;
		}

		void MeshSystem::LoadMesh(const PropertyElement& i_resource_element)
		{
			const std::string resource_name = i_resource_element.GetValue<std::string>("resource_name");
			const std::string path = i_resource_element.GetValue<std::string>("path");
			const std::string ver_usage = i_resource_element.GetValue<std::string>("vertices_usage");
			const std::string ind_usage = i_resource_element.GetValue<std::string>("indices_usage");
			BufferUsageFormat ver = StringToFormat(ver_usage);
			BufferUsageFormat ind = StringToFormat(ind_usage);
			LoadImpl(resource_name, path, ver, ind, "");
		}

		void MeshSystem::LoadModel(const PropertyElement& i_resource_element)
		{
			const std::string resource_name = i_resource_element.GetValue<std::string>("resource_name");
			const std::string path = i_resource_element.GetValue<std::string>("path");
			const std::string description_path = i_resource_element.GetValue<std::string>("description_path");
			const std::string ver_usage = i_resource_element.GetValue<std::string>("vertices_usage");
			const std::string ind_usage = i_resource_element.GetValue<std::string>("indices_usage");
			BufferUsageFormat ver = StringToFormat(ver_usage);
			BufferUsageFormat ind = StringToFormat(ind_usage);
			LoadImpl(resource_name, path, ver, ind, description_path);
		}
		
		MeshHandle MeshSystem::Load(const std::string& i_name, const std::string& i_path, Render::BufferUsageFormat i_vertices_usage, Render::BufferUsageFormat i_indices_usage)
		{
			return LoadImpl(i_name, i_path, i_vertices_usage, i_indices_usage, "");
		}

		MeshHandle MeshSystem::LoadImpl(const std::string& i_name, const std::string& i_path, BufferUsageFormat i_vertices_usage, BufferUsageFormat i_indices_usage, const std::string& i_desc_path)
		{
			MeshInformation info = { i_vertices_usage, i_indices_usage, i_desc_path };
			auto p_load_manager = Core::GetGlobalObject<Resources::ResourceManager>();
			InternalHandle handle = p_load_manager->Load<Mesh>(i_name, { i_path }, info);

			// resource is already loaded
			if (handle.index != -1)
			{
				assert(handle.index < static_cast<int>(m_handlers.size()));
				return m_handlers[handle.index];
			}

			return MeshHandle::InvalidHandle();
		}

		void MeshSystem::Unload(MeshHandle i_handler)
		{
			auto p_load_manager = Core::GetGlobalObject<Resources::ResourceManager>();
			p_load_manager->Unload<Mesh>({ i_handler.index, i_handler.generation });
		}

		void MeshSystem::Update(float i_elapsed_time)
		{

		}

		void MeshSystem::SubmitDrawCommands(Render::RenderWorld& i_render_world)
		{
			auto p_bucket = i_render_world.GetBucket(BucketType::Buffer);

			auto p_material_manager = Core::GetGlobalObject<Render::MaterialManager>();
			auto p_entity_manager = Core::GetGlobalObject<EntityManager>();
			assert(p_material_manager && "Material manager is not registered");
			assert(p_entity_manager && "Entity manager is not registered");

			for (auto& handler : m_component_handlers)
			{
				// reach the end of registered (valid) meshes
				if (handler.index == -1)
					break;

				auto& mesh_instance = m_instances[handler.index];
				auto& mesh = m_meshes[mesh_instance.GetHandler().index];
				auto p_entity = p_entity_manager->GetEntity(mesh_instance.GetEntity());
				for (size_t i = 0; i < mesh.GetSubmeshNumber(); ++i)
				{
					const Render::Mesh::SubMesh& sub_mesh = mesh.GetSubmesh(i);
					Commands::Transform* p_transform_cmd = nullptr;
					if (p_entity != nullptr)
					{
						auto p_transform = p_entity->GetComponent<Transform>();
						// we have no transform - cannot draw somewhere -> skip
						if (p_transform == nullptr)
						{
							assert(false && "No transform component for mesh");
							continue;
						}
						p_transform_cmd = p_bucket->Create<Commands::Transform>();
						p_transform_cmd->Translate(p_transform->m_position);
					}

					// TODO: need dynamic here and not static :`(
					// for now buffer place for 6 unis
					Commands::SetupShader<3, 30>* p_shader_cmd = p_bucket->Append<Commands::SetupShader<3, 30>>(p_transform_cmd);
					p_shader_cmd->m_layouts[0] = sub_mesh.m_pos_layout;
					p_shader_cmd->m_layouts[1] = sub_mesh.m_normal_layout;
					p_shader_cmd->m_layouts[2] = sub_mesh.m_uv_layout;

					void* p_parent_cmd = (void*)p_shader_cmd;
					if (!sub_mesh.m_materials.empty())
					{
						auto material_handle = sub_mesh.m_materials[0];
						const auto p_material = p_material_manager->AccessMaterial(material_handle);
						p_shader_cmd->m_shader = p_material->m_shader;
						for (auto& entry : p_material->m_entries)
						{
							if (entry.type != ShaderVariableType::Sampler2D)
								p_shader_cmd->SetValue(entry.shader_var_location, entry.type, entry.container.GetDataPtr(), entry.container.size, false);
						}

						p_parent_cmd = p_material_manager->SetupShaderAndCreateCommands(*p_bucket, &p_shader_cmd->m_dynamic_uniforms[p_shader_cmd->current_value], 6 - p_shader_cmd->current_value, *p_material, p_shader_cmd);
					}
					else
					{
						// TODO: dump material
					}
					// Add draw command
					Commands::Draw* p_cmd = p_bucket->Append<Commands::Draw>(p_parent_cmd);
					p_cmd->indices = sub_mesh.m_index_buffer;
				}
			}
		}

		MeshComponentHandle MeshSystem::CreateInstance(MeshHandle i_handle)
		{
			if (i_handle == MeshHandle::InvalidHandle() || i_handle.generation != m_handlers[i_handle.index].generation)
				return MeshComponentHandle::InvalidHandle();

			// get free index
			int new_index = -1;
			for (size_t i = 0; i < m_component_handlers.size(); ++i)
			{
				if (m_component_handlers[i].index == -1)
				{
					new_index = static_cast<int>(i);
					m_component_handlers[i].index = i;
					break;
				}
			}
			// create index
			if (new_index == -1)
			{
				// TODO: resize vector normally - with some strategy - not push_back
				const size_t new_count = m_component_handlers.size() + 1;
				m_component_handlers.resize(new_count);
				m_instances.resize(new_count);
				new_index = new_count - 1;
				m_component_handlers[new_index].generation = 0;
				m_component_handlers[new_index].index = new_index;				
			}
			
			//////////////////
			// set data for MeshComponent
			// TODO: increase use count for mesh
			m_instances[new_index] = MeshComponent(i_handle);			
			assert(m_instances.size() == m_component_handlers.size());
			return m_component_handlers[new_index];
		}

		MeshComponent* MeshSystem::AccessComponent(MeshComponentHandle i_handler)
		{
			if (m_component_handlers[i_handler.index].generation != i_handler.generation)
			{
				assert(false && "Generation is not identical");
				return nullptr;
			}
			return &m_instances[i_handler.index];
		}

		void MeshSystem::RemoveInstance(MeshComponentHandle i_handler)
		{
			// TODO: decrease use count for mesh
			m_component_handlers[i_handler.index].index = -1;
			++m_component_handlers[i_handler.index].generation;
		}

		///////////////////////////////////////////////////////////////////////////////
		// Extension for EntityManager

		MeshComponent* MeshSystem::Get(int i_in_system_id, int i_in_system_generation)
		{
			MeshComponentHandle inst_handler{ i_in_system_id, i_in_system_generation };
			MeshComponent* component = g_mesh_system.AccessComponent(inst_handler);
			return component;
		}

		void MeshSystem::Remove(int i_in_system_id, int i_in_system_generation)
		{
			g_mesh_system.RemoveInstance({ i_in_system_id, i_in_system_generation });
		}
		

	} // Render

} // SDK