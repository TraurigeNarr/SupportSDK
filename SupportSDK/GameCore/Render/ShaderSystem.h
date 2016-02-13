#ifndef	__GAMECORE_SHADERSYSTEM_H__
#define __GAMECORE_SHADERSYSTEM_H__

#include "../GameCoreAPI.h"
#include "RenderTypes.h"

#include "Shader.h"

namespace SDK
{
	namespace Resources {	namespace Serialization {
			template <typename ResInfo>
			struct LoaderImpl;
		} // Serialization
	} // Resources

	namespace Render
	{
		class ShaderSystem
		{
			friend struct Resources::Serialization::LoaderImpl<Shader>;
		private:
			// TODO: limitations to 1024 shader loaded - is it enough?
			typedef GenericBuffersArray<ShaderHandler, Shader, 1024> Shaders;
			Shaders m_shaders;

		public:
			ShaderSystem();
			struct ShaderSource
			{
				std::string vertex_shader_file;
				std::string tess_control_shader_file;
				std::string tess_compute_shader_file;
				std::string geometry_shader_file;
				std::string fragment_shader_file;
				std::string compute_shader_file;
				using ShaderSourceEntry = std::pair<Shader::ShaderType, std::string>;
				ShaderSource(std::initializer_list<ShaderSourceEntry> i_entries)
				{
					for (auto entry : i_entries)
					{
						switch (entry.first)
						{
							case Shader::Vertex:
								vertex_shader_file = std::move(entry.second);
								break;
							case Shader::TesselationConrol:
								tess_control_shader_file = std::move(entry.second);
								break;
							case Shader::TesselationEvaluation:
								tess_compute_shader_file = std::move(entry.second);
								break;
							case Shader::Geometry:
								geometry_shader_file = std::move(entry.second);
								break;
							case Shader::Fragment:
								fragment_shader_file = std::move(entry.second);
								break;
							case Shader::Compute:
								compute_shader_file = std::move(entry.second);
								break;
						}
					}
				}
				
			};
			GAMECORE_EXPORT ShaderHandler Load(const std::string& i_resource_name, ShaderSource i_source);
			GAMECORE_EXPORT void Unload(ShaderHandler i_handler);
			inline const Shader* Access(ShaderHandler i_handler) const
			{
				if (i_handler.index == -1 || i_handler.generation != m_shaders.m_handlers[i_handler.index].generation)
					return nullptr;
				return &m_shaders.m_buffer[i_handler.index];
			}
		};

		// TODO: global object
		GAMECORE_EXPORT extern ShaderSystem g_shader_system;

	} // Render

} // SDK

#endif
