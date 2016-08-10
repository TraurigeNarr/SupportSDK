#ifndef	__PATTERNS_FACTORY_H__
#define __PATTERNS_FACTORY_H__

#include <memory>
#include <vector>
#include <algorithm>

#include <Utilities/HashFunctions.h>

namespace SDK
{
	template <typename BaseType, typename IdentifierType, typename ReturnType>
	struct DefaultErrorPolicy
	{
		static ReturnType OnUnknownType(const IdentifierType& i_id)
		{
			return ReturnType();
		}

		static void OnCreatorExist(const IdentifierType& i_id)
		{
			// TODO: what default behavior should be: exception? log?
			return;
		}
	};

	template <
		typename BaseClass,
		typename IdentifierType,
		typename ReturnType = std::unique_ptr<BaseClass>,
		template <typename, typename, typename> class ErrorPolicy = DefaultErrorPolicy,
		size_t (*HashFunction)(const IdentifierType&) = &Utilities::hash_function<IdentifierType>,
		typename... CreatorParameters
	>
	class Factory
	{
	public:
		typedef ReturnType(*Creator)(CreatorParameters&&...);
		typedef ErrorPolicy<BaseClass, IdentifierType, ReturnType> _ErrorPolicy;
	private:
		typedef std::pair<size_t, Creator> ProductPair;
		std::vector<ProductPair> m_creators;

	private:
		const ProductPair* Find(size_t i_hash) const
		{
			const auto it = std::find_if(m_creators.begin(), m_creators.end(), [i_hash](const ProductPair& pp)
			{				
				return pp.first == i_hash;
			});
			if (it == m_creators.end())
				return nullptr;
			return &(*it);
		}

	public:		
		void Register(const IdentifierType& i_id, Creator i_creator)
		{
			const size_t hash = HashFunction(i_id);
			if (Find(hash) != nullptr)
				return _ErrorPolicy::OnCreatorExist(i_id);
					
			m_creators.emplace_back(hash, i_creator);
			std::sort(m_creators.begin(), m_creators.end(), [](const ProductPair& left, const ProductPair& right)
			{
				return left.first < right.first;
			});
		}

		void Unregister(const IdentifierType& i_id)
		{
			const size_t hash = HashFunction(i_id);
			m_creators.erase(
				std::find_if(m_creators.begin(), m_creators.end(), [hash](const ProductPair& p_p)
					{return p_p.first == hash; }),
					m_creators.end()
				);
		}

		ReturnType Create(const IdentifierType& i_id, CreatorParameters&&... i_parameters) const
		{
			const size_t hash = HashFunction(i_id);
			const ProductPair* p_p = Find(hash);
			if (p_p == nullptr)
				return _ErrorPolicy::OnUnknownType(i_id);
			return p_p->second(std::forward<CreatorParameters>(i_parameters...)...);
		}
	};


} // SDK

#endif