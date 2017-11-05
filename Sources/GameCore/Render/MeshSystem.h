#ifndef	__GAMECORE_MESHSYSTEM_H__
#define __GAMECORE_MESHSYSTEM_H__

#include "../GameCoreAPI.h"

#include "GlobalObjectBase.h"

#include "System.h"
#include "Render/Material.h"
#include "Render/Mesh.h"
#include "Render/MeshComponent.h"

#include "GenericHandlesDynamicArray.h"

#include "../PropertyElement.h"

namespace SDK
{
	struct MeshInformation;
	namespace Resources {
		namespace Serialization {

			template <typename ResType>
			struct Definition;
			template <>
			struct Definition <Render::Mesh>
			{
				typedef MeshInformation InfoType;
				typedef Render::MeshHandle HandleType;
			};

			template <typename ResInfo>
			struct LoaderImpl;

		}// Serialization
	} // Resources

	namespace Render
	{
		class MeshSystem : public System
						 , public GlobalObjectBase
		{
		private:
			friend struct Resources::Serialization::LoaderImpl<Mesh>;

		private:
			// raw resources (prototypes for dynamic meshes) - not processed if they not used with MeshComponent
			std::vector<Mesh> m_meshes;
			std::vector<MeshHandle> m_handlers;
			std::vector<MeshComponent> m_instances;
			// static mesh handles
			std::vector<MeshComponentHandle> m_component_handlers;

			using MeshHandles = GenericHandleDynamicArray<MeshHandle, Mesh>;
			MeshHandles m_dynamic_meshes;


		private:
			void LoadMesh(const PropertyElement& i_resource_element);
			void LoadModel(const PropertyElement& i_resource_element);

			MeshHandle LoadImpl(const std::string& i_name, const std::string& i_path, BufferUsageFormat i_vertices_usage, BufferUsageFormat i_indices_usage, const std::string& i_desc_path);

		public:
			MeshSystem();
			virtual ~MeshSystem();

			// System
			virtual void Update(float i_elapsed_time) override;
			virtual void SubmitDrawCommands(Render::RenderWorld& i_render_world) override;

			virtual bool Requires(Action i_aciton) const { return i_aciton == Action::SubmitDrawCommands || i_aciton == Action::Update; }

			// Own
			GAMECORE_EXPORT MeshHandle Load(const std::string& i_name, const std::string& i_path, BufferUsageFormat i_vertices_usage, BufferUsageFormat i_indices_usage);
			GAMECORE_EXPORT void Unload(MeshHandle i_handler);
			GAMECORE_EXPORT void Unload(const std::string& i_file_name) { throw std::exception("Not realized"); }
			
			GAMECORE_EXPORT MeshComponentHandle CreateInstance(MeshHandle i_handler, bool i_static_geometry);
			GAMECORE_EXPORT MeshComponentHandle CreateInstance(const std::string& i_file_name) { throw std::exception("Not realized"); }

			// TODO: Custom mesh
			//MeshHandle Create(const std::string& i_name, Mesh i_mesh);

			/////////////////////////////////////////////////////////
			
			// if you want to change something - use this method. On one frame it is guaranteed that 
			//	pointer will be valid if not nullptr
			GAMECORE_EXPORT MeshComponent* AccessComponent(MeshComponentHandle i_handler);
			GAMECORE_EXPORT void RemoveInstance(MeshComponentHandle i_handler);			

			void Initialize();
			void Release();

		// Extension for entity manager
		public:
			GAMECORE_EXPORT static MeshComponent* Get(int i_in_system_id, int i_in_system_generation);
			GAMECORE_EXPORT static void Remove(int i_in_system_id, int i_in_system_generation);
		};

	} // Render

} // SDK

#endif