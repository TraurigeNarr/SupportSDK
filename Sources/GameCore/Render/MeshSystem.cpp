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
	namespace Render
	{
		class AnimationSystem;

		struct Weight
		{
			size_t vertex_index;
			float weight;
		};
		struct Bone
		{
			std::string name;
			size_t name_hash;
			Matrix4f original_transform;
			std::vector<Weight> weights;

			Bone(const std::string& i_name, Matrix4f&& i_original_transform)
				: name(i_name)
				, name_hash(Utilities::hash_function(i_name))
				, original_transform(std::move(i_original_transform))
			{}
		};
		
		class Skeleton
		{
		private:
			using Bones = std::vector<Bone>;
			std::vector<Bones> m_bones;

			MeshHandle m_mesh;

		public:
			ENTITY_DEFINITION(AnimationSystem, "Skeleton")

			Skeleton() {}
			Skeleton(const Skeleton& i_other)
			{
				std::copy(i_other.m_bones.begin(), i_other.m_bones.end(), std::back_inserter(m_bones));
				m_mesh = i_other.m_mesh;
			}
			~Skeleton() {}

			std::vector<Bones>& GetBones() {
				return m_bones;
			}

			void SetMesh(MeshHandle i_mesh) {
				m_mesh = i_mesh;
			}

			MeshHandle GetMesh() {
				return m_mesh;
			}
		};

		template <typename _Key>
		struct AnimationKey
		{
			_Key value;
			float time = 0.0f;

			AnimationKey(const _Key& io_value, float i_time)
				: value(io_value)
				, time(i_time)
			{
			}
		};

		using Vector3Key = AnimationKey<Vector3>;
		using QuatKey = AnimationKey<QuaternionF>;

		class AnimationChannel
		{
		public:
			std::string m_affected_node_name;
			size_t m_name_hash;

			std::vector<Vector3Key> m_position_keys;
			std::vector<Vector3Key> m_scaling_keys;
			std::vector<QuatKey> m_rotation_keys;

			AnimationChannel(const std::string& i_name)
				: m_affected_node_name(i_name)
				, m_name_hash(Utilities::hash_function(i_name))
			{
			}
		};

		class Animation
		{
		public:
			real m_duration = 0.0f;
			real m_ticks_per_second = 0.0f;
			std::string m_name;
			size_t m_name_hash;

			std::vector<AnimationChannel> m_channels;

			bool m_active = false;

			Animation(const std::string& i_name, real i_duration, real i_ticks_per_sec)
				: m_name(i_name)
				, m_name_hash(Utilities::hash_function(i_name))
				, m_duration(i_duration)
				, m_ticks_per_second(i_ticks_per_sec)
			{
			}
		};

		class Animator
		{
		public:
			ENTITY_DEFINITION(AnimationSystem, "Animator")

		public:
			std::vector<Animation> m_animations;

			SkeletonHandle m_skeleton;
			MeshHandle m_mesh;
		};

		class AnimationSystem : public System
			, public GlobalObjectBase
		{
		private:
			using Skeletons = GenericHandleDynamicArray<SkeletonHandle, Skeleton>;
			Skeletons m_skeletons;

			using Animators = GenericHandleDynamicArray<AnimatorHandle, Animator>;
			Animators m_animators;

		public:
			// System
			virtual void Update(float i_elapsed_time) override;

			virtual bool Requires(Action i_aciton) const { return i_aciton == Action::Update; }

			std::pair<SkeletonHandle, Skeleton*> Create() {
				SkeletonHandle handle = m_skeletons.CreateNew();
				return { handle, m_skeletons.Access(handle) };
			}

			std::pair<SkeletonHandle, Skeleton*> CreateCopy(SkeletonHandle i_handle, MeshHandle i_mesh) {
				Skeleton* p_prototype = m_skeletons.Access(i_handle);
				SkeletonHandle handle = m_skeletons.CreateNew(*p_prototype);
				Skeleton* p_new = m_skeletons.Access(handle);
				p_new->SetMesh(i_mesh);
				return { handle, p_new };
			}

			Skeleton* Access(SkeletonHandle i_handle) {
				return m_skeletons.Access(i_handle);
			}
			void Remove(SkeletonHandle i_handle) {
				m_skeletons.Destroy(i_handle);
			}

			std::pair<AnimatorHandle, Animator*> CreateAnimator() {
				AnimatorHandle handle = m_animators.CreateNew();
				return { handle, m_animators.Access(handle) };
			}

			std::pair<AnimatorHandle, Animator*> CreateCopy(AnimatorHandle i_handle, MeshHandle i_mesh) {
				Animator* p_prototype = m_animators.Access(i_handle);
				AnimatorHandle handle = m_animators.CreateNew(*p_prototype);
				Animator* p_new = m_animators.Access(handle);
				p_new->m_mesh = i_mesh;
				return { handle, p_new };
			}

			Animator* Access(AnimatorHandle i_handle) {
				return m_animators.Access(i_handle);
			}
			void Remove(AnimatorHandle i_handle) {
				m_animators.Destroy(i_handle);
			}

			// Extension for entity manager
		public:
			GAMECORE_EXPORT static Skeleton* Get(int i_in_system_id, int i_in_system_generation);
			GAMECORE_EXPORT static void Remove(int i_in_system_id, int i_in_system_generation);
		};

		void AnimationSystem::Update(float i_elaped_time)
		{

		}


		///////////////////////////////////////////////////////////////////////////////
		// Extension for EntityManager

		Skeleton* AnimationSystem::Get(int i_in_system_id, int i_in_system_generation)
		{
			return Core::GetGlobalObject<Render::AnimationSystem>()->Access(SkeletonHandle{ i_in_system_id, i_in_system_generation });
		}

		void AnimationSystem::Remove(int i_in_system_id, int i_in_system_generation)
		{
			Core::GetGlobalObject<Render::AnimationSystem>()->Remove(SkeletonHandle{ i_in_system_id, i_in_system_generation });
		}

	}
}

namespace SDK
{
	struct MeshInformation
	{
		typedef Render::Mesh ResultType;

		Render::BufferUsageFormat m_ver_usage;
		Render::BufferUsageFormat m_ind_usage;
		std::string m_name;
		std::string m_description_path;
	};

	namespace Resources
	{	
		namespace Serialization
		{
			
			void ProcessSkeleton(Render::Skeleton& o_skeleton, const aiMesh& i_mesh, size_t i_submesh_index)
			{
				auto& submesh_bones = o_skeleton.GetBones();
				if (submesh_bones.size() <= i_submesh_index)
				{
					submesh_bones.resize(i_submesh_index);
				}
				auto& bones = submesh_bones[i_submesh_index];
				bones.reserve(i_mesh.mNumBones);
				for (size_t i = 0; i < i_mesh.mNumBones; ++i)
				{
					aiBone* p_aiBone = i_mesh.mBones[i];
					Render::Bone bone(p_aiBone->mName.C_Str(), {
						p_aiBone->mOffsetMatrix.a1, p_aiBone->mOffsetMatrix.a2, p_aiBone->mOffsetMatrix.a3, p_aiBone->mOffsetMatrix.a4,
						p_aiBone->mOffsetMatrix.b1, p_aiBone->mOffsetMatrix.b2, p_aiBone->mOffsetMatrix.b3, p_aiBone->mOffsetMatrix.b4,
						p_aiBone->mOffsetMatrix.c1, p_aiBone->mOffsetMatrix.c2, p_aiBone->mOffsetMatrix.c3, p_aiBone->mOffsetMatrix.c4,
						p_aiBone->mOffsetMatrix.d1, p_aiBone->mOffsetMatrix.d2, p_aiBone->mOffsetMatrix.d3, p_aiBone->mOffsetMatrix.d4,
					});
					bone.weights.reserve(p_aiBone->mNumWeights);
					for (size_t weight_idx = 0; weight_idx  < p_aiBone->mNumWeights; ++weight_idx)
					{
						Render::Weight w;
						w.vertex_index = p_aiBone->mWeights[weight_idx].mVertexId;
						w.weight = p_aiBone->mWeights[weight_idx].mWeight;
						bone.weights.emplace_back(w);
					}
					bones.push_back(bone);
				}
			}
		
			Render::AnimatorHandle ProcessAnimations(const aiScene& i_scene)
			{
				using namespace Render;
				if (!i_scene.HasAnimations())
					return Render::AnimatorHandle::InvalidHandle();
				std::pair<AnimatorHandle, Animator*> anim_pair = Core::GetGlobalObject<AnimationSystem>()->CreateAnimator();
				Animator& animator = *anim_pair.second;
				animator.m_animations.reserve(i_scene.mNumAnimations);
				for (size_t i = 0; i < i_scene.mNumAnimations; ++i)
				{
					const aiAnimation* p_anim = i_scene.mAnimations[i];
					Animation anim(p_anim->mName.C_Str(), p_anim->mDuration, p_anim->mTicksPerSecond);
					anim.m_channels.reserve(p_anim->mNumChannels);
					for (size_t channel_idx = 0; channel_idx < p_anim->mNumChannels; ++channel_idx)
					{
						const aiNodeAnim* p_channel = p_anim->mChannels[channel_idx];
						AnimationChannel channel(p_channel->mNodeName.C_Str());
						channel.m_position_keys.reserve(p_channel->mNumPositionKeys);
						for (size_t pos_idx = 0; pos_idx < p_channel->mNumPositionKeys; ++pos_idx)
						{
							const aiVectorKey& pos = p_channel->mPositionKeys[pos_idx];
							channel.m_position_keys.emplace_back(Vector3{ pos.mValue.x, pos.mValue.y, pos.mValue.z }, pos.mTime);
						}
						channel.m_rotation_keys.reserve(p_channel->mNumRotationKeys);
						for (size_t pos_idx = 0; pos_idx < p_channel->mNumRotationKeys; ++pos_idx)
						{
							const aiQuatKey& rot = p_channel->mRotationKeys[pos_idx];
							channel.m_rotation_keys.emplace_back(QuaternionF{ rot.mValue.x, rot.mValue.y, rot.mValue.z, rot.mValue.w }, rot.mTime);
						}
						channel.m_scaling_keys.reserve(p_channel->mNumScalingKeys);
						for (size_t pos_idx = 0; pos_idx < p_channel->mNumScalingKeys; ++pos_idx)
						{
							const aiVectorKey& scale = p_channel->mScalingKeys[pos_idx];
							channel.m_scaling_keys.emplace_back(Vector3{ scale.mValue.x, scale.mValue.y, scale.mValue.z }, scale.mTime);
						}
						
						anim.m_channels.push_back(std::move(channel));
					}

					animator.m_animations.push_back(std::move(anim));
				}
				return anim_pair.first;
			}

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
					const bool sequential = i_model_description.GetValue<bool>("sequential");
					auto p_load_manager = Core::GetGlobalObject<Resources::ResourceManager>();
					for (size_t i = 0; i < io_mesh.GetSubmeshNumber(); ++i)
					{
						Render::Mesh::SubMesh& sub_mesh = io_mesh.GetSubmesh(i);
						const size_t submesh_name = Utilities::hash_function(sub_mesh.m_name);
						auto it = std::find_if(materials.begin(), materials.end(), [submesh_name](const MeshMaterial& mat)
						{
							return mat.first == submesh_name;
						});
						if (sequential)
						{
							it = materials.begin();
							std::advance(it, i);
						}
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
					auto g_anim_sys = Core::GetGlobalObject<Render::AnimationSystem>();
					Render::Mesh result_mesh(i_info.m_name);

					const bool has_skeleton = ip_scene->HasAnimations();
					std::pair<Render::SkeletonHandle, Render::Skeleton*> skeleton_pair{ Render::SkeletonHandle::InvalidHandle(), nullptr };
					if (has_skeleton)
					{
						skeleton_pair = g_anim_sys->Create();
						result_mesh.SetSkeleton(skeleton_pair.first);
						skeleton_pair.second->GetBones().resize(ip_scene->mNumMeshes);
					}
					std::vector<Render::Vertex> vertices;
					std::vector<int> indices;
					auto p_mgr = Core::GetRenderer()->GetHardwareBufferMgr();
					
					volatile int vertex_count = 0;
					volatile int trig_count = 0;
					for (uint im = 0; im < ip_scene->mNumMeshes; ++im)
					{
						const aiMesh* p_mesh = ip_scene->mMeshes[im];
						p_mesh->mBones[0]->mName;
						p_mesh->mBones[0]->mWeights[0].mVertexId;
						if (!p_mesh->HasPositions())
						{
							assert(false && "Without positions");
							continue;
						}
						vertices.clear();
						vertices.reserve(p_mesh->mNumVertices);
						indices.clear();
						// Walk through each of the mesh's vertices
						for (uint i = 0; i < p_mesh->mNumVertices; i++)
						{
							Render::Vertex vertex;
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
							indices.reserve(indices.capacity() + face.mNumIndices);
							// Retrieve all indices of the face and store them in the indices vector
							for (uint j = 0; j < face.mNumIndices; j++)
								indices.push_back(face.mIndices[j]);
						}
						
						if (has_skeleton)
							ProcessSkeleton(*skeleton_pair.second, *p_mesh, im);

						auto ver_buf = p_mgr->CreateHardwareBuffer(vertices.size()*sizeof(Render::Vertex), i_info.m_ver_usage, &vertices[0]);
						Render::HardwareIndexBuffer::IndexType ind_type = Render::HardwareIndexBuffer::IndexType::Int;
						/*
						TODO: correct determination of vertex types
						if (indices.size() < 65535)
						ind_type = HardwareIndexBuffer::IndexType::Short;
						else if(indices.size() < 255)
						ind_type = HardwareIndexBuffer::IndexType::Byte;*/
						auto ind_buf = p_mgr->CreateIndexBuffer(ind_type, indices.size(), i_info.m_ind_usage, Render::PrimitiveType::Triangles, &indices[0]);
						auto pos_layout = p_mgr->CreateLayout(ver_buf, 3, Render::VertexSemantic::Position, Render::ComponentType::Float, false, sizeof(Render::Vertex), 0);
						auto normals_layout = p_mgr->CreateLayout(ver_buf, 3, Render::VertexSemantic::Normal, Render::ComponentType::Float, false, sizeof(Render::Vertex), offsetof(Render::Vertex, Normal));
						auto uv_layout = p_mgr->CreateLayout(ver_buf, 2, Render::VertexSemantic::TextureCoordinates, Render::ComponentType::Float, false, sizeof(Render::Vertex), offsetof(Render::Vertex, TexCoords));
						vertex_count += vertices.size();
						trig_count += indices.size() / 3;
						result_mesh.AddSubmesh(p_mesh->mName.C_Str(), ver_buf, pos_layout, normals_layout, uv_layout, ind_buf, vertices.size(), indices.size()/3);
						std::copy(vertices.begin(), vertices.end(), std::back_inserter(result_mesh.GetSubmesh(im).vertices));
					}

					Render::AnimatorHandle animator_handle = ProcessAnimations(*ip_scene);
					result_mesh.SetAnimator(animator_handle);

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
						return std::make_pair(LoadResult::Failure, Render::Mesh(i_info.m_name));
					}
					// Read file via ASSIMP
					Assimp::Importer importer;
					const aiScene* scene = importer.ReadFileFromMemory(file_data.c_str(), file_data.size(), aiProcessPreset_TargetRealtime_Fast);
					// Check for errors
					if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
					{
						std::string message = importer.GetErrorString();
						assert(false && "ASSIMP ERROR");
						return std::make_pair(LoadResult::Failure, Render::Mesh(i_info.m_name));
					}

					return ProcessMesh(scene, i_info);
				}

				static InternalHandle CreateNewHandle()
				{
					auto handle = Core::GetGlobalObject<Render::MeshSystem>()->m_raw_meshes.CreateNew();
					return { handle.index, handle.generation };
				}

				static void RemoveHandle(InternalHandle i_handle)
				{
					Core::GetGlobalObject<Render::MeshSystem>()->m_raw_meshes.Destroy({ i_handle.index, i_handle.generation });
				}

				static void UnloadResource(InternalHandle i_handle)
				{
					auto p_mesh_system = Core::GetGlobalObject<Render::MeshSystem>();
					// release resources for mesh
					auto p_mgr = Core::GetRenderer()->GetHardwareBufferMgr();

					Render::Mesh* p_mesh = p_mesh_system->m_raw_meshes.Access({ i_handle.index, i_handle.generation });
					assert(p_mesh);
					for (size_t i = 0; i < p_mesh->GetSubmeshNumber(); ++i)
					{
						const Render::Mesh::SubMesh& sub_mesh = p_mesh->GetSubmesh(i);
						p_mgr->DestroyBuffer(sub_mesh.m_vertex_buffer);
						p_mgr->DestroyLayout(sub_mesh.m_pos_layout);
						p_mgr->DestroyLayout(sub_mesh.m_normal_layout);
						p_mgr->DestroyLayout(sub_mesh.m_uv_layout);
						p_mgr->DestroyBuffer(sub_mesh.m_index_buffer);
					}
					p_mesh_system->m_raw_meshes.Destroy({ i_handle.index, i_handle.generation });				
				}

				static void Register(InternalHandle i_handle, Render::Mesh i_mesh)
				{
					Core::GetGlobalObject<Render::MeshSystem>()->m_raw_meshes.m_elements[i_handle.index].second = std::move(i_mesh);
				}
			};
		
		} // Serialization

	} // Resources

	namespace Render
	{

		MeshSystem::MeshSystem()
		{
			// TODO: remove this dirty hack
			Core::GetGlobalObjectStorage()->AddGlobalObject<Render::AnimationSystem>();
		}

		MeshSystem::~MeshSystem()
		{
			Core::GetGlobalObjectStorage()->RemoveGlobalObject<Render::AnimationSystem>();
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
			else if (i_format == "dynamic")
				return BufferUsageFormat::Dynamic;
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
			MeshInformation info = { i_vertices_usage, i_indices_usage, i_name, i_desc_path };
			auto p_load_manager = Core::GetGlobalObject<Resources::ResourceManager>();
			InternalHandle handle = p_load_manager->Load<Mesh>(i_name, { i_path }, info);

			// resource is already loaded
			if (handle.index != -1)
			{
				SkeletonHandle skeleton = m_raw_meshes.Access({ handle.index, handle.generation })->GetSkeleton();
				AnimatorHandle animator = m_raw_meshes.Access({ handle.index, handle.generation })->GetAnimator();
				if (Skeleton* p_skeleton = Core::GetGlobalObject<AnimationSystem>()->Access(skeleton))
					p_skeleton->SetMesh(MeshHandle{ handle.index, handle.generation });
				if (Animator* p_animator = Core::GetGlobalObject<AnimationSystem>()->Access(animator))
					p_animator->m_mesh = MeshHandle{ handle.index, handle.generation };
				assert(handle.index < static_cast<int>(m_raw_meshes.m_elements.size()));
				return m_raw_meshes.m_elements[handle.index].first;
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
			m_vertices_rendered = 0;
			m_triangles_rendered = 0;
			for (auto& mesh_instance_pair : m_mesh_instances.m_elements)
			{
				// reach the end of registered (valid) meshes
				if (mesh_instance_pair.first.index == -1)
					break;
				auto& mesh_instance = mesh_instance_pair.second;
				Mesh* p_mesh = mesh_instance.IsStaticGeometry() ? m_raw_meshes.Access(mesh_instance.GetHandler()) : m_dynamic_meshes.Access(mesh_instance.GetHandler());
				// TODO: dump mesh
				if (p_mesh == nullptr)
				{
					assert(false);
					continue;
				}
				auto p_entity = p_entity_manager->GetEntity(mesh_instance.GetEntity());
				for (size_t i = 0; i < p_mesh->GetSubmeshNumber(); ++i)
				{
					const Render::Mesh::SubMesh& sub_mesh = p_mesh->GetSubmesh(i);
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
						p_transform_cmd->Rotate(p_transform->m_rotation);
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
						size_t setuped = 0;
						p_parent_cmd = p_material_manager->SetupShaderAndCreateCommands(*p_bucket, &p_shader_cmd->m_dynamic_uniforms[p_shader_cmd->current_value], setuped, 6 - p_shader_cmd->current_value, *p_material, p_shader_cmd);
						p_shader_cmd->current_value += setuped;
					}
					else
					{
						// TODO: dump material
					}

					Commands::SetSubdata* p_set_subdata = p_bucket->Append<Commands::SetSubdata>(p_parent_cmd);
					// TODO: offset = 0 -> need one blob of binary data and SubMesh::offset
					p_set_subdata->Set((void*)&sub_mesh.vertices[0], sub_mesh.vertices.size(), 0, sub_mesh.m_vertex_buffer);
					
					// Add draw command
					Commands::Draw* p_cmd = p_bucket->Append<Commands::Draw>(p_set_subdata);
					p_cmd->indices = sub_mesh.m_index_buffer;

					m_vertices_rendered += sub_mesh.m_vertices_count;
					m_triangles_rendered += sub_mesh.m_triangles_count;
				}
			}
		}

		MeshComponentHandle MeshSystem::CreateInstance(MeshHandle i_handle, bool i_static_geometry)
		{
			if (!m_raw_meshes.IsValid(i_handle))
				return MeshComponentHandle::InvalidHandle();

			Mesh* p_mesh = m_raw_meshes.Access(i_handle);
			if (p_mesh == nullptr)
			{
				assert(p_mesh);
				return MeshComponentHandle::InvalidHandle();
			}
			MeshHandle handle_to_mesh = i_static_geometry ? i_handle : m_dynamic_meshes.CreateNew(*p_mesh);
			return m_mesh_instances.CreateNew(handle_to_mesh, i_static_geometry);
		}

		MeshComponent* MeshSystem::AccessComponent(MeshComponentHandle i_handler)
		{
			return m_mesh_instances.Access(i_handler);
		}

		void MeshSystem::RemoveInstance(MeshComponentHandle i_handler)
		{
			m_mesh_instances.Destroy(i_handler);
		}

		MeshHandle MeshSystem::AccessMesh(const std::string& i_name)
		{
			size_t name_hash = Utilities::hash_function(i_name);
			auto it = std::find_if(m_raw_meshes.m_elements.begin(), m_raw_meshes.m_elements.end(), [name_hash](MeshHandles::ArrayElement& element)
			{return element.second.GetNameHash() == name_hash; });
			return it == m_raw_meshes.m_elements.end() ? MeshHandle::InvalidHandle() : it->first;
		}

		///////////////////////////////////////////////////////////////////////////////
		// Extension for EntityManager

		MeshComponent* MeshSystem::Get(int i_in_system_id, int i_in_system_generation)
		{
			MeshComponentHandle inst_handler{ i_in_system_id, i_in_system_generation };
			MeshComponent* component = Core::GetGlobalObject<Render::MeshSystem>()->AccessComponent(inst_handler);
			return component;
		}

		void MeshSystem::Remove(int i_in_system_id, int i_in_system_generation)
		{
			Core::GetGlobalObject<Render::MeshSystem>()->RemoveInstance({ i_in_system_id, i_in_system_generation });
		}
		

	} // Render

} // SDK