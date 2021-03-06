#ifndef	__GAMECORE_FONTMANAGER_H__
#define __GAMECORE_FONTMANAGER_H__

#include "GameCoreAPI.h"

#include "GlobalObjectBase.h"

#include "Font.h"
#include "GenericHandlesStaticArray.h"

#include "PropertyElement.h"

namespace SDK
{
	namespace Resources {
		namespace Serialization {

			template <typename ResType>
			struct Definition;
			template <>
			struct Definition <Render::Font>
			{
				typedef Render::FontSettings InfoType;
				typedef Render::FontHandle HandleType;
			};

			template <typename ResInfo>
			struct LoaderImpl;

		}// Serialization
	} // Resources

	namespace Render
	{
		class FontManager : public GlobalObjectBase
		{
		private:
			friend struct Resources::Serialization::LoaderImpl<Font>;

		private:
			// TODO: compile with MAX_FONTS_NUMBER
			using FontsArray = GenericHandlesStaticArray<FontHandle, Font, 64>;
			FontsArray m_fonts;

		private:
			void Load(const PropertyElement& i_resource_element);

		public:
			GAMECORE_EXPORT FontHandle LoadFont(const std::string& i_resource_name, FontSettings i_settings, const std::string& i_file_name);
			GAMECORE_EXPORT void Unload(FontHandle i_texture);
			GAMECORE_EXPORT Font* Access(FontHandle i_texture);

			GAMECORE_EXPORT void Render(Vector2 i_position, float i_scale, const std::wstring& i_text, FontHandle i_font);

			void Initialize();
			void Release();
		};

	} // Render
} // SDK

#endif