#include "stdafx.h"

#include "GLTextureManager.h"

#include "Core.h"
#include "Render/IRenderer.h"
#include "Resources/ResourceManager.h"
#include "Render/OpenGL/GlUitlities.h"

#include "FileSystem/FileSystem.h"

#include <SOIL.h>

#include <GL/glew.h>

namespace SDK
{
	namespace Resources
	{
		namespace Serialization
		{
			template <>
			struct LoaderImpl < Render::Texture >
			{
				static std::pair<LoadResult, Render::Texture> Load(std::istream* ip_streams[1], void*)
				{
					const std::string file_data = FS::ReadFileToString(*ip_streams[0]);
					if (file_data.empty())
					{
						assert(false && "[Textures] File data is empty");
						return std::make_pair(LoadResult::Failure, Render::Texture());
					}

					int width, height;

					unsigned char* image = SOIL_load_image_from_memory(reinterpret_cast<const unsigned char*>(file_data.c_str()),file_data.size(), &width, &height, 0, SOIL_LOAD_RGB);
					if (image == nullptr)
						return std::make_pair(LoadResult::Failure, Render::Texture());

					GLuint texture;
					glGenTextures(1, &texture);
					glBindTexture(GL_TEXTURE_2D, texture);
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
					glGenerateMipmap(GL_TEXTURE_2D);
					// Parameters
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glBindTexture(GL_TEXTURE_2D, 0);

					SOIL_free_image_data(image);

					Render::Texture t;
					t.hardware_id = texture;
					t.type = Render::TextureType::TwoD;
					t.image_format = Render::ImageFormat::RGBA;

					return std::make_pair(LoadResult::Success, t);
				}

				static std::pair<LoadResult, Render::Texture> Load(Render::RawTexture&& i_data)
				{
					if (i_data.mp_data == nullptr)
						return std::make_pair(LoadResult::Failure, Render::Texture());
					GLuint texture;
					glGenTextures(1, &texture);
					glBindTexture(GL_TEXTURE_2D, texture);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

					glTexImage2D(
						GL_TEXTURE_2D,
						0,
						GL_RED,
						i_data.m_width,
						i_data.m_height,
						0,
						GL_RED,
						GL_UNSIGNED_BYTE,
						i_data.mp_data.get()
						);
					glBindTexture(GL_TEXTURE_2D, 0);

					Render::Texture t;
					t.hardware_id = texture;
					t.type = Render::TextureType::TwoD;
					t.image_format = Render::ImageFormat::RGBA;

					return std::make_pair(LoadResult::Success, t);
				}

				static InternalHandle CreateNewHandle()
				{
					Render::GLTextureManager* p_mgr = static_cast<Render::GLTextureManager*>(Core::GetRenderer()->GetTextureManager());
					Render::TextureHandle handle = p_mgr->m_textures.Create();

					return{ handle.index, handle.generation };
				}

				static void RemoveHandle(InternalHandle i_handle)
				{
					Render::GLTextureManager* p_mgr = static_cast<Render::GLTextureManager*>(Core::GetRenderer()->GetTextureManager());
					p_mgr->m_textures.Destroy({ i_handle.index, i_handle.generation });
				}

				static void UnloadResource(InternalHandle i_handle)
				{
					Render::GLTextureManager* p_mgr = static_cast<Render::GLTextureManager*>(Core::GetRenderer()->GetTextureManager());
					p_mgr->m_textures.Destroy({ i_handle.index, i_handle.generation });

				}

				static void Register(InternalHandle i_handle, Render::Texture i_texture)
				{
					Render::GLTextureManager* p_mgr = static_cast<Render::GLTextureManager*>(Core::GetRenderer()->GetTextureManager());
					p_mgr->m_textures.m_elements[i_handle.index].second = i_texture;
				}
			};

		} // Serialization
	} // Resources

	namespace Render
	{
		GLTextureManager::~GLTextureManager()
		{
		}

		void GLTextureManager::Initialize()
		{
			auto p_load_manager = Core::GetGlobalObject<Resources::ResourceManager>();
			p_load_manager->RegisterLoader<GLTextureManager, Texture>(*this, &GLTextureManager::Load, "texture");
		}

		void GLTextureManager::Release()
		{
			auto p_load_manager = Core::GetGlobalObject<Resources::ResourceManager>();
			p_load_manager->Unregister<GLTextureManager, Texture>("texture");
		}

		void GLTextureManager::Load(const PropertyElement& i_resource_element)
		{
			const std::string resource_name = i_resource_element.GetValue<std::string>("resource_name");
			const std::string path = i_resource_element.GetValue<std::string>("path");
			Load(resource_name, path);
		}

		TextureHandle GLTextureManager::Load(const std::string& i_resource_name, const std::string & i_file_name)
		{
			auto p_load_manager = Core::GetGlobalObject<Resources::ResourceManager>();
			InternalHandle handle = p_load_manager->Load<Texture>(i_resource_name, { i_file_name }, nullptr);
			// resource is already loaded
			if (handle.index != -1)
			{
				assert(handle.index < static_cast<int>(TexturesArray::Size));
				return m_textures.m_elements[handle.index].first;
			}

			return TextureHandle::InvalidHandle();
		}

		TextureHandle GLTextureManager::Load(const std::string& i_resource_name, RawTexture i_texture)
		{
			auto p_load_manager = Core::GetGlobalObject<Resources::ResourceManager>();

			InternalHandle handle = p_load_manager->LoadFromData<Texture>(i_resource_name, std::move(i_texture));
			// resource is already loaded
			if (handle.index != -1)
			{
				assert(handle.index < static_cast<int>(TexturesArray::Size));
				return m_textures.m_elements[handle.index].first;
			}

			return TextureHandle::InvalidHandle();
		}

		Texture* GLTextureManager::Access(TextureHandle i_texture)
		{
			return m_textures.Access(i_texture);
		}

		void GLTextureManager::Unload(TextureHandle i_texture)
		{
			auto p_load_manager = Core::GetGlobalObject<Resources::ResourceManager>();
			p_load_manager->Unload<Texture>({ i_texture.index, i_texture.generation });
		}

		void GLTextureManager::Bind(int i_target, TextureHandle i_handle)
		{
			const auto p_texture = m_textures.Access(i_handle);
			if (p_texture == nullptr || p_texture->hardware_id == -1)
			{
				// TODO: insert blank texture
				assert(false && "No texture");
				return;
			}

			glActiveTexture(GL_TEXTURE0 + i_target);	// Activate the texture unit first before binding texture
			// TODO: p_texture->type
			glBindTexture(GL_TEXTURE_2D, p_texture->hardware_id);
			CHECK_GL_ERRORS;
		}

		void GLTextureManager::Release(int i_target)
		{
			glActiveTexture(GL_TEXTURE0 + i_target);
			glBindTexture(GL_TEXTURE_2D, 0);
			CHECK_GL_ERRORS;
		}

	} // Render
} // SDK