#include "stdafx.h"

#include "FontManager.h"

#include "Resources/ResourceManager.h"
#include "FileSystem/FileSystem.h"
#include "Core.h"
#include "Render/IRenderer.h"
#include "Render/ShaderSystem.h"

#include "Render/OpenGL/GlUitlities.h"
#include "Render/HardwareBufferManagerBase.h"

#include "Render/TextureAtlas.h"
#include "Render/TextureManager.h"

#include <ft2build.h>
#include FT_FREETYPE_H

// TODO: temporary in GL
#include <GL/glew.h>

#include <SOIL.h>

namespace SDK
{
	namespace Resources
	{
		namespace Serialization
		{

			template <>
			struct LoaderImpl < Render::Font >
			{
				static Render::Font Preprocess(Render::TextureAtlas& o_atlas, FT_Face& io_face, const Render::FontSettings& i_settings)
				{
					Render::Font font;
					font.m_name = i_settings.name;
					o_atlas.Clear();
					FT_GlyphSlot slot;
					FT_Bitmap ft_bitmap;
					int ft_glyph_top = 0;
					int ft_glyph_left = 0;
					int missed = 0;
					FT_Set_Pixel_Sizes(io_face, 0, 48);
					for (int code : i_settings.symbols_to_load)
					{
						FT_UInt glyph_index = FT_Get_Char_Index(io_face, code);
						if (FT_Load_Glyph(io_face, glyph_index, FT_LOAD_RENDER))
						{
							assert(false && "ERROR::FREETYTPE: Failed to load Glyph");
							continue;
						}

						slot = io_face->glyph;
						ft_bitmap = slot->bitmap;
						if (ft_bitmap.buffer == nullptr)
							continue;
						ft_glyph_top = slot->bitmap_top;
						ft_glyph_left = slot->bitmap_left;
						int w = ft_bitmap.width / o_atlas.Deepth();
						int h = ft_bitmap.rows;
						auto region = o_atlas.GetNewRegion(w + 1, h + 1);
						auto bl = region.GetBottomLeft();
						if (bl[0] < 0)
						{
							++missed;
							//assert(false && "Missed character in font");
							continue;
						}
						o_atlas.SetRegion(region, ft_bitmap.buffer, ft_bitmap.pitch);
						Render::Font::Character ch;
						ch.char_id = code;
						ch.Size = { w, h };
						ch.Bearing = { ft_glyph_left, ft_glyph_top };
						ch.UV_bot_left = { bl[0] / (float)o_atlas.Width(), bl[1] / (float)o_atlas.Height() };
						ch.UV_top_right = { (bl[0] + w) / float(o_atlas.Width()), (bl[1] + h) / float(o_atlas.Height()) };

						// Discard hinting to get advance
						FT_Load_Glyph(io_face, glyph_index, FT_LOAD_RENDER | FT_LOAD_NO_HINTING);
						slot = io_face->glyph;
						// Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
						ch.Advance = { slot->advance.x >> 6, slot->advance.y >> 6 };
						
						font.m_characters.push_back(ch);
					}
					return font;
				}

				static std::pair<LoadResult, Render::Font> Load(std::istream* ip_streams[1], Render::FontSettings i_settings)
				{
					if (i_settings.symbols_to_load.empty() || i_settings.pixel_height == 0 || i_settings.name.empty())
						return std::make_pair(LoadResult::Failure, Render::Font());

					FT_Library ft;
					FT_Face face;

					if (FT_Init_FreeType(&ft))
					{
						assert(false && "ERROR::FREETYPE: Could not init FreeType Library");
						FT_Done_FreeType(ft);
						return std::make_pair(LoadResult::Failure, Render::Font());
					}
					
					const std::string font_data = FS::ReadFileToString(*ip_streams[0]);
					if (FT_New_Memory_Face(ft, reinterpret_cast<const FT_Byte*>(font_data.c_str()), font_data.size(), 0, &face))
					{
						assert(false && "ERROR::FREETYPE: Failed to load font");
						FT_Done_FreeType(ft);
						FT_Done_Face(face);
						return std::make_pair(LoadResult::Failure, Render::Font());
					}
					Render::TextureAtlas atlas(512, 256, 1);
					Render::Font target_font = Preprocess(atlas, face, i_settings);
#if defined(_DEBUG)
					//SOIL_save_image("temp.bmp", SOIL_SAVE_TYPE_BMP, atlas.Width(), atlas.Height(), 1, atlas.GetDatPtr());
#endif

					FT_Done_Face(face);
					FT_Done_FreeType(ft);

					target_font.m_texture_handle = Core::GetRenderer()->GetTextureManager()->Load(target_font.m_name, atlas.Release());

					return std::make_pair(LoadResult::Success, target_font);
				}

				// TODO: need some limitations on size of loaded meshes?
				static InternalHandle CreateNewHandle()
				{
					Render::FontHandle handle = Core::GetGlobalObject<Render::FontManager>()->m_fonts.Create();
					return{ handle.index, handle.generation };
				}

				static void RemoveHandle(InternalHandle i_handle)
				{
					Core::GetGlobalObject<Render::FontManager>()->m_fonts.Destroy({ i_handle.index, i_handle.generation });
				}

				static void UnloadResource(InternalHandle i_handle)
				{
					Core::GetGlobalObject<Render::FontManager>()->m_fonts.Destroy({ i_handle.index, i_handle.generation });
				}

				static void Register(InternalHandle i_handle, Render::Font i_font)
				{
					Core::GetGlobalObject<Render::FontManager>()->m_fonts.m_elements[i_handle.index].second = i_font;
				}
			};

		} // Serialization
	} // Resources

	namespace Render
	{
		void FontManager::Load(const PropertyElement & i_resource_element)
		{
			const std::string path = i_resource_element.GetValue<std::string>("path");
			FontSettings settings;
			settings.name = i_resource_element.GetValue<std::string>("resource_name");
			settings.symbols_to_load = i_resource_element.GetValue<std::wstring>("codes");
			settings.pixel_width = i_resource_element.GetValue<int>("pixel_width");
			settings.pixel_height = i_resource_element.GetValue<int>("pixel_height");
			if (settings.symbols_to_load.empty())
				return;
			LoadFont(settings.name, settings, path);
		}

		FontHandle FontManager::LoadFont(const std::string& i_resource_name, FontSettings i_settings, const std::string& i_file_name)
		{
			auto p_load_manager = Core::GetGlobalObject<Resources::ResourceManager>();
			InternalHandle handle = p_load_manager->Load<Font>(i_resource_name, { i_file_name }, i_settings);
			if (handle.index != -1)
			{
				assert(handle.index < static_cast<int>(FontsArray::Size));
				return m_fonts.m_elements[handle.index].first;
			}

			return FontHandle::InvalidHandle();
		}
		
		void FontManager::Render(Vector2 i_position, float i_scale, const std::wstring& i_text, FontHandle i_font)
		{
			auto p_font = m_fonts.Access(i_font);
			if (p_font == nullptr)
				return;
			auto p_renderer = Core::GetRenderer();
			auto shader = Core::GetGlobalObject<Render::ShaderSystem>()->Access("TextShader");
			auto p_load_manager = Core::GetGlobalObject<Resources::ResourceManager>();
			auto shader_handle = p_load_manager->GetHandleToResource<Shader>("TextShader");

			p_renderer->Bind(shader_handle, nullptr, 0);
			Core::GetGlobalObject<Render::ShaderSystem>()->SetKnownUniforms(shader_handle);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			auto p_hd_mgr = p_renderer->GetHardwareBufferMgr();			
			auto buffer_handle = p_hd_mgr->CreateHardwareBuffer(sizeof(float) * 6 * 4, BufferUsageFormat::Dynamic);
			p_hd_mgr->BindBuffer(buffer_handle);
			CHECK_GL_ERRORS;
			auto layout_handle = p_hd_mgr->CreateLayout(buffer_handle, 4, VertexSemantic::Position, ComponentType::Float, false, 4 * sizeof(float), 0);

			int location = shader->GetAttribute("position").location;
			p_hd_mgr->BindLayout(layout_handle, location);
			CHECK_GL_ERRORS;
			glEnableVertexAttribArray(location);
			glUniform3f(shader->GetUniform("textColor").location, 1.f, 0.f, 0.f);
			CHECK_GL_ERRORS;
			// Render glyph texture over quad
			

			//////////////////////
			// render text
			constexpr int space_width = 25;
			p_renderer->GetTextureManager()->Bind(0, p_font->m_texture_handle);
			for (int c : i_text)
			{
				if (c == L' ')
				{
					i_position[0] += space_width * i_scale;
					continue;
				}

				Font::Character ch = p_font->Find(c);
				if (ch.char_id == -1)
					continue;

				GLfloat xpos = i_position[0] + ch.Bearing[0] * i_scale;
				GLfloat ypos = i_position[1] - (ch.Size[1] - ch.Bearing[1]) * i_scale;

				GLfloat w = ch.Size[0] * i_scale;
				GLfloat h = ch.Size[1] * i_scale;
				// Update VBO for each character
				GLfloat vertices[6][4] = {
					{ xpos,     ypos + h,   ch.UV_bot_left[0], ch.UV_bot_left[1] },
					{ xpos,     ypos,       ch.UV_bot_left[0],  ch.UV_top_right[1] },
					{ xpos + w, ypos,        ch.UV_top_right[0], ch.UV_top_right[1] },

					{ xpos,     ypos + h,   ch.UV_bot_left[0], ch.UV_bot_left[1] },
					{ xpos + w, ypos,        ch.UV_top_right[0], ch.UV_top_right[1] },
					{ xpos + w, ypos + h,    ch.UV_top_right[0], ch.UV_bot_left[1] }
				};
				

				p_hd_mgr->SetSubdata(buffer_handle, vertices, 0, sizeof(vertices));
				p_hd_mgr->BindBuffer(buffer_handle);
				// Render quad
				glDrawArrays(GL_TRIANGLES, 0, 6);
				// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
				i_position[0] += ch.Advance[0] * i_scale;
			}

			p_hd_mgr->DestroyBuffer(buffer_handle);
			p_hd_mgr->DestroyLayout(layout_handle);
			p_renderer->GetTextureManager()->Release(0);

			p_renderer->UnbindShader();
		}

		void FontManager::Initialize()
		{
			auto p_load_manager = Core::GetGlobalObject<Resources::ResourceManager>();
			p_load_manager->RegisterLoader<FontManager, Font>(*this, &FontManager::Load, "font");
		}

		void FontManager::Release()
		{
			auto p_load_manager = Core::GetGlobalObject<Resources::ResourceManager>();
			p_load_manager->Unregister<FontManager, Font>("font");
		}

	} // Render
} // SDK