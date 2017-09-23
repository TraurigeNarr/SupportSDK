#include "stdafx.h"

#include "CoreDelegateImpl.h"

#include <GameCore/Core.h>
#include <GameCore/Render/IRenderer.h>
#include <GameCore/Render/HardwareBufferManagerBase.h>
#include <GameCore/Input/inputsystem.h>

// for test
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#include <time.h>

#include <Math/Matrix4.h>

using namespace SDK;

#include <GameCore/Applications/ApplicationBase.h>

#include <GameCore/UI/UIControlSystem.h>
#include <Patterns/MessageDispatcher/Connection.h>
#include <string>

#include <GameCore/Resources/ResourceManager.h>
#include <GameCore/Render/ShaderCompiler.h>
#include <GameCore/Render/Fonts/FontManager.h>

#include <GameCore/Logging/LogSystem.h>
#include <GameCore/Logging/BasicPolicies.h>
#include <Utilities/StringUtilities.h>

namespace Game
{

	struct TestHandler
	{
		void Handle(const UI::UIButtonEvent& e)
		{
			if (e.m_type == UI::UIButtonEvent::Type::Released)
			{
				Core::GetApplication()->RequestShutdown();
			}
		}
	};
	TestHandler handler;

	struct SSS
	{
		const char* file_name;
		const char* function_name;
		const int line;
		SSS(const char* file = __FILE__, int l = __LINE__)
			: file_name(file)
			, function_name("")
			, line(l)
		{}
	};

	void CoreDelegateImpl::OnCreate()
	{
		auto p_load_manager = Core::GetGlobalObject<Resources::ResourceManager>();
		p_load_manager->LoadResourceSet("..\\..\\..\\Samples\\Resources\\ResourceSets\\ui_testing.res");

		auto p_ui_system = Core::GetGlobalObject<UI::UIControlSystem>();
		p_ui_system->LoadScheme("..\\..\\..\\Samples\\Resources\\UI\\test.scheme");
		p_ui_system->UnloadScheme("test");
		p_ui_system->SetActiveScheme("test");
		auto scheme_handler = p_ui_system->LoadScheme("..\\..\\..\\Samples\\Resources\\UI\\TestUIProfile.scheme");				
		p_ui_system->SetActiveScheme(scheme_handler);
		auto accessor = p_ui_system->CreateControl<UI::UIButton>();
		p_ui_system->RemoveControl(accessor.GetHandler());
		auto accessor1 = p_ui_system->CreateControl<UI::UIButton>();

		auto& msg_dsp = p_ui_system->GetMessageDispatcher();
		msg_dsp.RegisterHandler<TestHandler, UI::UIButtonEvent>(handler, &TestHandler::Handle, "exit_button");

		//connection = SDK::Connection(msg_dsp, handler, &TestHandler::Handle, "my_mega_button");		
		//connection.disconnect();

		Log::Info("Render", Source, "Test message");
		Log::Info(0, Source, "{0}: {1}", "asdasd", 1);
		Log::Info(0, Source, "asdfasdf");
	}

	void CoreDelegateImpl::OnTerminate()
	{
	}
	static std::wstring message_num_0 = L"asdqwetbij[we6  ewrgasdf";
	void CoreDelegateImpl::Update(float i_elapsed_time)
	{
		if (Core::GetGlobalObject<InputSystem>()->GetKeyboard().IsKeyPressed(SDK::Key::A))
			message_num_0 = L"AbBbCcDdEeFfGgHh";
		else
			message_num_0 = L"A released!#!Arj";
	}
	
	void CoreDelegateImpl::Draw()
	{
		static float x = 50;
		static float y = 600;
		auto p_renderer = Core::GetRenderer();
		IRect rect = p_renderer->GetTargetRectangle();
		p_renderer->SetMatrix(MatrixMode::Projection, Matrix4f::CreateOrtho(0, rect.Width(), 0, rect.Height()));
		std::wstring message = L"FPS: " + std::to_wstring(Core::GetApplication()->GetCurrentFPS());
		auto en_font = Core::GetGlobalObject<Resources::ResourceManager>()->GetHandleToResource<Render::Font>("Arial_en");
		auto arial_font = Core::GetGlobalObject<Resources::ResourceManager>()->GetHandleToResource<Render::Font>("Arial");
		Core::GetGlobalObject<Render::FontManager>()->Render({ x, y }, 1.f, message, arial_font);
		Core::GetGlobalObject<Render::FontManager>()->Render({ x, y - 50 }, 1.f, message_num_0, en_font);
		Core::GetGlobalObject<Render::FontManager>()->Render({ x, y - 100 }, 1.f, L"Мама мыла раму!", arial_font);
	}

} // Game