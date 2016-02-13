#ifndef __GAMECORE_MESHCOMPONENT_H__
#define __GAMECORE_MESHCOMPONENT_H__

#include "Mesh.h"

#include "../Component.h"
#include "../Entity.h"

namespace SDK
{

	namespace Render
	{
		class MeshSystem;
		
		class MeshComponent
		{
		public:
			ENTITY_DEFINITION(MeshSystem, "MeshComponent")

		private:
			MeshHandler m_mesh;

			EntityHandle m_entity;

			// set of materials
			std::vector<MaterialHandle> m_materials;

		public:
			MeshComponent() {}
			MeshComponent(MeshHandler i_handler)
				: m_mesh(i_handler)
			{}			

			MeshHandler GetHandler() const { return m_mesh; }

			void AddMaterial(MaterialHandle i_material) { m_materials.emplace_back(i_material); }
			const std::vector<MaterialHandle>& GetMaterials() const { return m_materials; }

			void SetEntity(EntityHandle i_entity) { m_entity = i_entity; }
			EntityHandle GetEntity() const { return m_entity; }
		};

	} // Render

} // SDK

#endif