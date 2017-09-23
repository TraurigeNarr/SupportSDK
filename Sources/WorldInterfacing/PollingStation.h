#ifndef __POLLINGSTATION_H__
#define __POLLINGSTATION_H__

#include "PollingTask.h"
#include "TypeDefines.h"

#include <exception>
#include <memory>
#include <set>

namespace SDK
	{
	
	template <typename PollingType>
	struct PollingHashStrategy_ByCode
			{
			typedef MetaPollingTask<PollingType> PollingTask;
			typedef std::unique_ptr<PollingTask> PollingTaskPtr;

			bool operator () (const PollingTaskPtr& i_first, const PollingTaskPtr& i_second) const
				{
				return i_first->GetTaskCode() < i_second->GetTaskCode();
				}
			};

	template <typename PollingType, typename HashStrategy = PollingHashStrategy_ByCode<PollingType>>
	class MetaPollingStation
		{
		public:
			typedef PollingType PollingType;
			typedef HashStrategy HashStrategy;
			typedef MetaPollingTask<PollingType> PollingTask;
			typedef std::unique_ptr<PollingTask> PollingTaskPtr;

		private:
			struct TaskFinder
				{
				uint m_code_to_find;
				TaskFinder(uint i_code_to_find) : m_code_to_find(i_code_to_find){}
				bool operator () (const PollingTaskPtr& i_task) const { return i_task->GetTaskCode() == m_code_to_find; }
				};

		private:
			std::set< PollingTaskPtr, HashStrategy > m_tasks;

		public:
			MetaPollingStation () { }
			~MetaPollingStation() { }

			void RegisterTask (PollingTaskPtr ip_task)
				{
				m_tasks.insert( std::move(ip_task) );
				}

			PollingType Poll (uint i_code)
				{
				#ifdef _DEBUG
				if (std::find_if(m_tasks.begin(), m_tasks.end(), TaskFinder(i_code)) == m_tasks.end())
					{
					throw std::runtime_error("[MetaPollingStation] Task is not registered.");
					}
				#endif
				return ( *std::find_if(m_tasks.begin(), m_tasks.end(), TaskFinder(i_code)) )->GetValue();
				}
		};

	} // SDK

#endif