#ifndef	__GAMECORE_DEFAULTGLOBALOBJECTGETTER_H__
#define __GAMECORE_DEFAULTGLOBALOBJECTGETTER_H__

#include "GameCoreAPI.h"

#include "ObjectGetterBase.h"

// objects
#include "Resources/ResourceManager.h"

namespace SDK
{

	class DefaultGlobalObjectGetter : public ObjectGetterBase
	{
	private:
		std::vector<GlobalObjectBase*> m_cache_objects;
		using ObjPtr = std::unique_ptr<GlobalObjectBase>;
		std::vector<ObjPtr> m_dynamic_objects;

		// static objects
		Resources::ResourceManager m_resource_manager;

	private:
		virtual GlobalObjectBase* GetGlobalObjectImpl(size_t i_type_code) const override;
		virtual void AddGlobalObjectImpl(std::unique_ptr<GlobalObjectBase> ip_object) override;
		virtual void RemoveGlobalObjectImpl(size_t i_type_code) override;

	public:
		GAMECORE_EXPORT DefaultGlobalObjectGetter();
	};

} // SDK

#endif
