#ifndef __GAMECORE_PROPERTYREADERS_H__
#define __GAMECORE_PROPERTYREADERS_H__

#include "GameCoreAPI.h"

#include "PropertyReader.h"

namespace SDK
{

	template <>
	class PropretyReader <(int)ReaderType::XML>
	{
	public:
		GAMECORE_EXPORT PropertyElement Parse(const std::string& i_file_name) const;
		GAMECORE_EXPORT PropertyElement Parse(std::istream& i_stream) const;
	};

	template <>
	class PropretyReader <(int)ReaderType::SDKFormat>
	{
	public:
		GAMECORE_EXPORT PropertyElement Parse(const std::string& i_file_name) const;
		GAMECORE_EXPORT PropertyElement Parse(std::istream& i_stream) const;
	};

	template <>
	class PropretyReader <(int)ReaderType::Binary>
	{
	public:
		GAMECORE_EXPORT PropertyElement Parse(const std::string& i_file_name) const;
		GAMECORE_EXPORT PropertyElement Parse(std::istream& i_stream) const;
	};

} // SDK

#endif
