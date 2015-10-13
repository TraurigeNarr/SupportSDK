#ifndef __GAMECODE_INPUTSYSTEM_H__
#define __GAMECODE_INPUTSYSTEM_H__

#include "GameCoreAPI.h"

#include "Core.h"
#include "Input/InputEvent.h"

#include <Patterns/Singleton.h>

#if defined(_USRDLL)
#pragma warning ( push )
#pragma warning( disable : 4251 )
#endif

namespace SDK
{
	class InputSubscriber;

	class GAMECORE_EXPORT InputSystem : public Singleton<InputSystem>
	{
		friend void Core::CreateSingletons();
	private:
		InputSystem();
		virtual ~InputSystem();

	private:
		std::vector<InputSubscriber*> m_subscribers;

	public:
		void ProcessEvent(const InputEvent& i_event);

		void AddSubscriber(InputSubscriber* ip_subscriber);
		void RemoveSubscriber(InputSubscriber* ip_subscriber);
	};

} // SDK

#if defined(_USRDLL)
#pragma warning ( pop )
#endif

#endif