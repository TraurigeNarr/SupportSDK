#ifndef	__GAMECORE_TRANSFORMATIONSYSTEM_H__
#define __GAMECORE_TRANSFORMATIONSYSTEM_H__

#include "../GameCoreAPI.h"

#include "GlobalObjectBase.h"

#include "../Component.h"
#include "../Entity.h"
#include "../System.h"

#include "../GenericHandle.h"

namespace SDK
{

	class TransformationsSystem;

	struct Transform
	{
	public:
		ENTITY_DEFINITION(TransformationsSystem, "Transform");

	public:
		Vector3 m_position;
		Matrix3 m_rotation;
		// rotation
		// scale
		// http://bitsquid.blogspot.com/2012/07/matrices-rotation-scale-and-drifting.html

		Transform()
		{
			m_rotation.Identity();
		}
		Transform(Vector3 i_position)
			: m_position(i_position)
		{
			m_rotation.Identity();
		}

		// Extension for EntityManager
		EntityHandle m_entity;
		void SetEntity(EntityHandle i_entity) { m_entity = i_entity; }
		EntityHandle GetEntity() const { return m_entity; }
	};

	struct TransformHandlerTag {};
	typedef GenericHandle<int, 12, 20, TransformHandlerTag> TransformHandler;

	class TransformationsSystem : public System
								, public GlobalObjectBase
	{
	private:
		// separate vectors for trnasform;aabb and other, or one handler and {transform;aabb;etc.}
		std::vector<TransformHandler> m_handlers;		
		// TODO: vector or ?
		std::vector<Transform> m_transforms;

		// aabbs
		// partition

	public:
		// System
		virtual void Update(float i_elapsed_time) override;

		virtual bool Requires(Action i_aciton) const { return i_aciton == Action::Update; }

		GAMECORE_EXPORT TransformHandler CreateInstance();
		// if you want to change something - use this method. On one frame it is guaranteed that 
		//	pointer will be valid if not nullptr
		GAMECORE_EXPORT Transform* GetInstance(TransformHandler i_handler);
		GAMECORE_EXPORT void RemoveInstance(TransformHandler i_handler);

		// TODO: Custom mesh
		//MeshHandle Register(const std::string& i_name, Mesh i_mesh);
	// Extension for entity manager
	public:
		GAMECORE_EXPORT static Transform* Get(int i_in_system_id, int i_in_system_generation);
		GAMECORE_EXPORT static void Remove(int i_in_system_id, int i_in_system_generation);
	};

} // SDK

#endif
