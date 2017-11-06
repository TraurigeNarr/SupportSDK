#include "stdafx.h"

#include "Commands.h"

#include "CommandPacket.h"

#include "Core.h"
#include "IRenderer.h"

#include "Render/ShaderSystem.h"
#include "Render/LightsController.h"
#include "Render/TextureManager.h"
#include "Render/HardwareBufferManagerBase.h"
namespace SDK
{
	namespace Render
	{
		namespace Commands
		{

			///////////////////////////////////////////////////////////////////////////
			// Draw

			void SetSubdata::SetDefaultValues()
			{
				data = nullptr;
				data_offset = 0;
				data_size = 0;
				vertices = VertexBufferHandle::InvalidHandle();
			}

			void SetSubdata::Setup(const void* ip_data)
			{
				const SetSubdata* cmd = reinterpret_cast<const SetSubdata*>(ip_data);
				auto mgr = Core::GetRenderer()->GetHardwareBufferMgr();
				mgr->SetSubdata(cmd->vertices, cmd->data, cmd->data_offset, cmd->data_size);
			}

			///////////////////////////////////////////////////////////////////////////
			// Draw

			void Draw::SetDefaultValues()
			{
				vertices = VertexBufferHandle::InvalidHandle();
				layout = VertexLayoutHandle::InvalidHandle();
				indices = IndexBufferHandle::InvalidHandle();
				startVertex = 0;
				vertexCount = 0;
			}

			void Draw::DrawFunction(const void* ip_data)
			{
				auto p_renderer = Core::GetRenderer();
				const Draw* cmd = reinterpret_cast<const Draw*>(ip_data);

				// TODO: setup uniforms

				p_renderer->Draw({ cmd->vertices,cmd->layout,cmd->indices });
			}

			///////////////////////////////////////////////////////////////////////////
			// Transform

			void Transform::SetDefaultValues()
			{
				m_position[0] = 0.f; m_position[1] = 0.f; m_position[2] = 0.f;
				m_scale[0] = 1.f; m_scale[1] = 1.f; m_scale[2] = 1.f;
				for (size_t i = 0; i < 9; ++i)
					m_rotation[i] = i / 3 == i % 3 ? 1.f : 0.f;
			}

			void Transform::PushTransformation(const void* ip_data)
			{
				auto p_renderer = Core::GetRenderer();
				const Transform* cmd = reinterpret_cast<const Transform*>(ip_data);

				p_renderer->PushMatrix();

				Matrix4f transformation;
				transformation.MakeTransform(
				{ cmd->m_position[0], cmd->m_position[1], cmd->m_position[2] },
				{ cmd->m_scale[0], cmd->m_scale[1], cmd->m_scale[2] },
					Matrix3(cmd->m_rotation));

				p_renderer->ModifyCurrentMatrix(transformation);
			}

			void Transform::PopTransformation(const void* ip_data)
			{
				auto p_renderer = Core::GetRenderer();
				p_renderer->PopMatrix();
			}

			///////////////////////////////////////////////////////////////////////////
			// SetLight

			void SetLights::ApplyLights(const void* ip_data)
			{
				const SetLights* cmd = reinterpret_cast<const SetLights*>(ip_data);
				auto p_lights = Core::GetRenderer()->GetLightsController();

				cmd->m_was_enabled = p_lights->LightingEnabled();

				if (cmd->m_enabled)
					p_lights->EnableLighting();
				else
					p_lights->DisableLighting();
			}

			void SetLights::RestoreLightConfig(const void* ip_data)
			{
				const SetLights* cmd = reinterpret_cast<const SetLights*>(ip_data);
				auto p_lights = Core::GetRenderer()->GetLightsController();

				if (cmd->m_was_enabled)
					p_lights->EnableLighting();
				else
					p_lights->DisableLighting();
			}

			void SetLights::SetDefaultValues()
			{
				m_enabled = false;
				m_was_enabled = false;
				m_handle = INVALID_LIGHT_HANDLE;
			}

			///////////////////////////////////////////////////////////////////////////
			// SetupShader

			namespace SetupShaderDetails
			{
				void BindShader(ShaderHandle i_shader, const VertexLayoutHandle* i_layouts, size_t i_size)
				{
					Core::GetRenderer()->Bind(i_shader, i_layouts, i_size);
					Core::GetGlobalObject<Render::ShaderSystem>()->SetKnownUniforms(i_shader);
				}

				void UnbindShader()
				{
					Core::GetRenderer()->UnbindShader();
				}

				void SetDynamicUniform(const ShaderUniformValue& i_value)
				{
					Core::GetGlobalObject<Render::ShaderSystem>()->SetUniform(i_value.location, i_value);
				}
			} // SetupShaderDetails

			///////////////////////////////////////////////////////////////////////////
			// BindTexture

			void BindTexture::SetDefaultValues()
			{
				target = 0;
				texture_handle = InternalHandle::InvalidHandle();
			}

			void BindTexture::Bind(const void* ip_data)
			{
				auto p_renderer = Core::GetRenderer();
				const BindTexture* p_cmd = reinterpret_cast<const BindTexture*>(ip_data);

				auto p_tex_mgr = p_renderer->GetTextureManager();
				p_tex_mgr->Bind(p_cmd->target, { p_cmd->texture_handle.index, p_cmd->texture_handle.generation });
			}

			void BindTexture::Release(const void* ip_data)
			{
				auto p_renderer = Core::GetRenderer();
				const BindTexture* p_cmd = reinterpret_cast<const BindTexture*>(ip_data);

				auto p_tex_mgr = p_renderer->GetTextureManager();
				p_tex_mgr->Release(p_cmd->target);
			}

		} // Commands
	} // Render
} // SDK