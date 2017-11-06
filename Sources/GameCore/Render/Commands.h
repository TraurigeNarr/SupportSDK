#ifndef	__GAMECORE_COMMANDS_H__
#define __GAMECORE_COMMANDS_H__

#include "../Render/RenderTypes.h"
#include "../Render/Shader.h"

namespace SDK
{
	namespace Render
	{
		namespace Commands
		{
			struct SetSubdata
			{
				IMPLEMENT_COMMAND_EXPORT(Setup);

				void* data;
				size_t data_size;
				size_t data_offset;
				VertexBufferHandle vertices;

				void Set(void* p_data, size_t i_data_size, size_t i_data_offset, VertexBufferHandle i_handle)
				{
					data = p_data;
					data_size = i_data_size;
					data_offset = i_data_offset;
					vertices = i_handle;
				}
			};
			static_assert(std::is_pod<SetSubdata>::value == true, "SetSubdata must be a POD.");

			struct Draw
			{
				IMPLEMENT_COMMAND_EXPORT(DrawFunction);

				// COMMAND OWN MEMBERS
				uint vertexCount;
				uint startVertex;

				VertexBufferHandle		vertices;
				VertexLayoutHandle		layout;
				IndexBufferHandle		indices;
			};
			static_assert(std::is_pod<Draw>::value == true, "Draw must be a POD.");

			struct Transform
			{
				IMPLEMENT_COMMAND_WITH_COMPLETION_EXPORT(PushTransformation, PopTransformation);

				enum Actions
				{
					TRANSLATE = 1 << 0,
					SCALE = 1 << 1,
					ROTATE = 1 << 2
				};

				int m_actions;
				float m_position[3];
				float m_scale[3];
				float m_rotation[3 * 3];		

				inline void Translate(Vector3& i_position)
				{
					memcpy(m_position, i_position.GetPointer(), sizeof(float) * 3);
					m_actions |= TRANSLATE;
				}
				inline void Scale(Vector3& i_scale)
				{
					memcpy(m_scale, i_scale.GetPointer(), sizeof(float) * 3);
					m_actions |= SCALE;
				}
				inline void Rotate(Matrix3& i_rotation)
				{
					memcpy(m_rotation, i_rotation[0], sizeof(float) * 3 * 3);
					m_actions |= ROTATE;
				}
			};
			static_assert(std::is_pod<Transform>::value == true, "Transform must be a POD.");

			struct SetLights
			{
				IMPLEMENT_COMMAND_WITH_COMPLETION_EXPORT(ApplyLights, RestoreLightConfig);

				bool m_enabled;
				LightHandle m_handle;

				mutable bool m_was_enabled;
			};
			static_assert(std::is_pod<SetLights>::value == true, "SetLights must be a POD.");

			namespace SetupShaderDetails
			{
				GAMECORE_EXPORT void BindShader(ShaderHandle i_shader, const VertexLayoutHandle* i_layouts, size_t i_size);
				GAMECORE_EXPORT void UnbindShader();
				GAMECORE_EXPORT void SetDynamicUniform(const ShaderUniformValue& i_value);
			} // SetupShaderDetails
			template <size_t layouts_count, size_t dynamic_uniforms_count = 1>
			struct SetupShader
			{
				static const size_t LayoutsCount = layouts_count;
				static const size_t DynUniCount = dynamic_uniforms_count;

				static void Bind(const void* ip_data)
				{
					const SetupShader<LayoutsCount, DynUniCount>* cmd = reinterpret_cast<const SetupShader<LayoutsCount, DynUniCount>*>(ip_data);
					SetupShaderDetails::BindShader(cmd->m_shader, &cmd->m_layouts[0], LayoutsCount);
					for (size_t i = 0; i < cmd->current_value; ++i)
						SetupShaderDetails::SetDynamicUniform(cmd->m_dynamic_uniforms[i]);
				}
				static void Unbind(const void*)
				{
					SetupShaderDetails::UnbindShader();
				}
				void SetDefaultValues()
				{
					current_value = 0;
					m_shader.index = -1;
					for (auto& val : m_dynamic_uniforms)
						val.Reset();
				}
				IMPLEMENT_COMMAND_IMPL(Bind, Unbind);
				
				///////////////////////////////////////////////////
				// Own

				ShaderHandle m_shader;
				std::array<VertexLayoutHandle, LayoutsCount> m_layouts;
				std::array<ShaderUniformValue, DynUniCount> m_dynamic_uniforms;
				size_t current_value;

				template <typename T>
				void SetValue(int i_location, ShaderVariableType i_type, const T& i_value, bool i_transposed)
				{				
					SetValue(i_location, i_type, &i_value, sizeof(T), i_transposed);
				}

				template <typename T>
				void SetValue(int i_location, ShaderVariableType i_type, const T& i_value)
				{
					SetValue(i_location, i_type, i_value, false);
				}

				inline void SetValue(int i_location, ShaderVariableType i_type, const void* ip_raw_data, size_t i_size, bool i_transposed)
				{
					if (current_value >= DynUniCount)
					{
						assert(false && "Try to set more than allocated");
						return;
					}
					m_dynamic_uniforms[current_value] = ShaderUniformValue::Construct(i_location, i_type, ip_raw_data, i_size, i_transposed);

					++current_value;
				}
			};
			
			static_assert(std::is_pod<SetupShader<1>>::value == true, "SetupShader must be a POD.");

			struct BindTexture
			{
				IMPLEMENT_COMMAND_WITH_COMPLETION_EXPORT(Bind, Release);

				int target;
				InternalHandle texture_handle;
			};

		} // Commands
	} // Render
} // SDK

#endif
