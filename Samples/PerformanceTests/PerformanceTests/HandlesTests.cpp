#include "stdafx.h"

#include "HandlesTests.h"

#include "PerformanceCommon.h"

#include <GameCore/GenericHandlesStaticArray.h>
#include <GameCore/GenericHandlesDynamicArray.h>

#include <random>

using namespace SDK;

namespace HandlesTests
{
	enum Action
	{
		Static = 0,
		Dynamic = 1,
		Raw = 2
	};
	class Aggregator
	{
	private:
		using ActionPair = std::pair<void*, int>;
		using Actions = std::vector<ActionPair>;
		Actions m_actions[3];
	public:
		void MakeAction(void* ip_actor, int i_action_type)
		{
			Actions& actions = m_actions[i_action_type];
			auto it = std::find_if(actions.begin(), actions.end(), [ip_actor](const ActionPair& action)
			{
				return action.first == ip_actor;
			});

			if (it == actions.end())
			{
				actions.emplace_back(std::make_pair(ip_actor, 1));
				return;
			}
			++it->second;
		}

		void OutResults()
		{
			std::cout << "Actors [Static]: " << m_actions[0].size() << std::endl
				<< "Actors [Dynamic]: " << m_actions[1].size() << std::endl
				<< "Actors [Raw]: " << m_actions[2].size() << std::endl;
		}

		void Clear()
		{
			m_actions[0].clear();
			m_actions[1].clear();
			m_actions[2].clear();
		}
	};
	static Aggregator g_aggregator;

	class ConcreteResource
	{
	public:
		ConcreteResource()
		{}

		void MakeAction(int i_action)
		{
			g_aggregator.MakeAction(this, i_action);
		}
	};

	namespace test_impl
	{
		template <typename Manager>
		std::vector<typename Manager::OutputHandle> FillManager(Manager& o_manager, int i_number)
		{
			std::vector<typename Manager::OutputHandle> result;
			result.reserve(i_number);
			for (int i = 0; i < i_number; ++i)
			{
				result.emplace_back(o_manager.CreateEntry());
			}
			return result;
		}

		template <typename Manager>
		void AccessManagerConsequentially(Manager& i_manager, const std::vector<typename Manager::OutputHandle>& i_handles)
		{
			for (Manager::OutputHandle handle : i_handles)
				Manager::Access(i_manager, handle)->MakeAction(Manager::ActionType);
		}

		template <typename Manager>
		void AccessManagerRandomly(Manager& i_manager, const std::vector<typename Manager::OutputHandle>& i_handles)
		{
			for (Manager::OutputHandle handle : i_handles)
			{
				Manager::Access(i_manager, handle)->MakeAction(Manager::ActionType);
			}
		}

		template <typename Manager>
		void ReleaseManager(Manager& o_manager)
		{
			o_manager.Release();
		}
	} // test_impl
	
	namespace StaticArray
	{
		struct ConcreteHandleTag {};
		using ConcreteHandle = SDK::GenericHandle<int, 12, 20, ConcreteHandleTag>;

		class Manager
		{
		public:
			using OutputHandle = ConcreteHandle;
			static constexpr int ActionType = Action::Static;
			static ConcreteResource* Access(Manager& i_manager, ConcreteHandle i_handle)
			{
				return i_manager.Access(i_handle);
			}
		private:
			SDK::GenericHandlesStaticArray<ConcreteHandle, ConcreteResource, 1024> m_resources;
		public:

			ConcreteResource* Access(ConcreteHandle i_handle)
			{
				return m_resources.Access(i_handle);
			}

			ConcreteHandle CreateEntry()
			{
				return m_resources.Create();
			}

			void Release()
			{
				m_resources.ClearAll();
			}
		};
	} // StaticArray

	namespace DynamicArray
	{
		struct ConcreteHandleTag {};
		using ConcreteHandle = SDK::GenericHandle<int, 20, 12, ConcreteHandleTag>;

		class Manager
		{
		public:
			using OutputHandle = ConcreteHandle;
			static constexpr int ActionType = Action::Dynamic;
			static ConcreteResource* Access(Manager& i_manager, ConcreteHandle i_handle)
			{
				return i_manager.Access(i_handle);
			}
		private:
			SDK::GenericHandleDynamicArray<ConcreteHandle, ConcreteResource> m_resources;
		public:

			ConcreteResource* Access(ConcreteHandle i_handle)
			{
				return m_resources.Access(i_handle);
			}

			ConcreteHandle CreateEntry()
			{
				return m_resources.CreateNew();
			}

			void Release()
			{
				m_resources.ClearAll();
			}
		};
	} // DynamicArray

	namespace RawPointer
	{
		class Manager
		{
		public:
			using OutputHandle = ConcreteResource*;
			static constexpr int ActionType = Action::Raw;
			static ConcreteResource* Access(Manager& i_manager, ConcreteResource* i_handle)
			{
				return i_handle;
			}
		private:
			std::vector<std::unique_ptr<ConcreteResource>> m_resources;

		public:
			ConcreteResource* CreateEntry()
			{
				m_resources.emplace_back(std::make_unique<ConcreteResource>());
				return m_resources.back().get();
			}

			void Release()
			{
				m_resources.clear();
			}
		};
	} // RawPointer

	using namespace PerformanceTests;

	void Test_StaticDynamic(int i_entries, int i_conseq, int i_random)
	{
		std::cout << "-----------------------------------" << std::endl;
		std::cout << "\tStatic and dynamic" << std::endl;
		StaticArray::Manager static_manager;
		DynamicArray::Manager dynamic_manager;

		std::vector<StaticArray::Manager::OutputHandle> static_results;
		std::vector<DynamicArray::Manager::OutputHandle> dynamic_results;

		// Fill
		clock_t dynamic_manager_fill = Runner([&dynamic_manager, &dynamic_results, i_entries]() {dynamic_results = test_impl::FillManager(dynamic_manager, i_entries); }, 1);
		clock_t static_manager_fill = Runner([&static_manager, &static_results, i_entries]() {static_results = test_impl::FillManager(static_manager, i_entries); }, 1);
		std::cout << "Fill: " << std::endl
			<< "\tDynamic: " << dynamic_manager_fill << std::endl
			<< "\tRaw: " << static_manager_fill << std::endl;

		// Access (consequentially)
		clock_t dynamic_manager_conseq = Runner([&dynamic_manager, &dynamic_results]() {test_impl::AccessManagerConsequentially(dynamic_manager, dynamic_results); }, i_conseq);
		clock_t static_manager_conseq = Runner([&static_manager, &static_results]() {test_impl::AccessManagerConsequentially(static_manager, static_results); }, i_conseq);
		std::cout << "Access(conseq): " << std::endl
			<< "\tDynamic: " << dynamic_manager_conseq << std::endl
			<< "\tStatic: " << static_manager_conseq << std::endl;

		// Access (randomly)
		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(dynamic_results.begin(), dynamic_results.end(), g);
		std::shuffle(static_results.begin(), static_results.end(), g);
		clock_t dynamic_manager_random = Runner([&dynamic_manager, &dynamic_results]() {test_impl::AccessManagerRandomly(dynamic_manager, dynamic_results); }, i_random);
		clock_t static_manager_random = Runner([&static_manager, &static_results]() {test_impl::AccessManagerRandomly(static_manager, static_results); }, i_random);
		std::cout << "Access(random): " << std::endl
			<< "\tDynamic: " << dynamic_manager_random << std::endl
			<< "\tStatic: " << static_manager_random << std::endl;

		clock_t dynamic_manager_release = Runner([&dynamic_manager]() {test_impl::ReleaseManager(dynamic_manager); }, 1);
		clock_t raw_manager_release = Runner([&static_manager]() {test_impl::ReleaseManager(static_manager); }, 1);
		std::cout << "Release: " << std::endl
			<< "\tDynamic: " << dynamic_manager_release << std::endl
			<< "\tStatic: " << raw_manager_release << std::endl;

		clock_t static_manager_release = Runner([&static_manager]() {test_impl::ReleaseManager(static_manager); }, 1);
	}

	void Test_DynamicRaw(int i_entries, int i_conseq, int i_random)
	{
		std::cout << "-----------------------------------" << std::endl;
		std::cout << "\tDynamic and raw" << std::endl;

		DynamicArray::Manager dynamic_manager;
		RawPointer::Manager raw_manager;

		std::vector<DynamicArray::Manager::OutputHandle> dynamic_results;
		std::vector<RawPointer::Manager::OutputHandle> raw_results;

		// Fill
		clock_t dynamic_manager_fill = Runner([&dynamic_manager, &dynamic_results, i_entries]() {dynamic_results = test_impl::FillManager(dynamic_manager, i_entries); }, 1);
		clock_t raw_manager_fill = Runner([&raw_manager, &raw_results, i_entries]() {raw_results = test_impl::FillManager(raw_manager, i_entries); }, 1);
		std::cout << "Fill: " << std::endl
			<< "\tDynamic: " << dynamic_manager_fill << std::endl
			<< "\tRaw: " << raw_manager_fill << std::endl;

		// Access (consequentially)
		clock_t dynamic_manager_conseq = Runner([&dynamic_manager, &dynamic_results]() {test_impl::AccessManagerConsequentially(dynamic_manager, dynamic_results); }, i_conseq);
		clock_t raw_manager_conseq = Runner([&raw_manager, &raw_results]() {test_impl::AccessManagerConsequentially(raw_manager, raw_results); }, i_conseq);
		std::cout << "Access(conseq): " << std::endl
			<< "\tDynamic: " << dynamic_manager_conseq << std::endl
			<< "\tRaw: " << raw_manager_conseq << std::endl;

		// Access (randomly)
		clock_t dynamic_manager_random = Runner([&dynamic_manager, &dynamic_results]() {test_impl::AccessManagerRandomly(dynamic_manager, dynamic_results); }, i_random);
		clock_t raw_manager_random = Runner([&raw_manager, &raw_results]() {test_impl::AccessManagerRandomly(raw_manager, raw_results); }, i_random);
		std::cout << "Access(random): " << std::endl
			<< "\tDynamic: " << dynamic_manager_random << std::endl
			<< "\tRaw: " << raw_manager_random << std::endl;

		clock_t dynamic_manager_release = Runner([&dynamic_manager]() {test_impl::ReleaseManager(dynamic_manager); }, 1);
		clock_t raw_manager_release = Runner([&raw_manager]() {test_impl::ReleaseManager(raw_manager); }, 1);
		std::cout << "Release: " << std::endl
			<< "\tDynamic: " << dynamic_manager_release << std::endl
			<< "\tRaw: " << raw_manager_release << std::endl;
	}

	void Test()
	{
		std::cout << "-----------------------------------" << std::endl;
		std::cout << "Handles tests" << std::endl;

		Test_StaticDynamic(1024, 1000, 1000);
		g_aggregator.OutResults();
		g_aggregator.Clear();

		static constexpr int ENTRIES_COUNT = 10000;
		static constexpr int CONSEQUENTIALLY_PASSES = 100;
		static constexpr int RANDOM_PASSES = 100;
		Test_DynamicRaw(ENTRIES_COUNT, CONSEQUENTIALLY_PASSES, RANDOM_PASSES);
		g_aggregator.OutResults();
		g_aggregator.Clear();

		std::cout << "-----------------------------------" << std::endl;
	}

} // HandlesTests