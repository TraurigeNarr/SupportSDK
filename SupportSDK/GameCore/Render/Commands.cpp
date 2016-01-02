#include "stdafx.h"

#include "Commands.h"

#include "CommandPacket.h"

#include "Core.h"
#include "IRenderer.h"

#include <GL/glew.h>
#include "Render/ShaderSystem.h"

namespace SDK
{
	namespace Render
	{
		namespace Commands
		{
			///////////////////////////////////////////////////////////////////////////
			// Draw
			
			void Draw::SetDefaultValues()
			{
				vertices.index = 0;
				layout.index = 0;
				indices.index = 0;
				program.index = -1;
				startVertex = 0;
				vertexCount = 0;
			}

			void Draw::DrawFunction(const void* ip_data)
			{
				auto p_renderer = Core::GetRenderer();
				const Draw* cmd = reinterpret_cast<const Draw*>(ip_data);

				auto shader = g_shader_system.Access(cmd->program);
				//if (shader.IsValid())					
				//	glUseProgram(shader.GetId());

				Batch b;
				b.vertices = cmd->vertices;
				b.element = cmd->layout;
				b.indices = cmd->indices;
				
				p_renderer->Draw(b);

				//if (shader.IsValid())
				//	glUseProgram(0);
			}

			///////////////////////////////////////////////////////////////////////////
			// Draw

			void Transform::SetDefaultValues()
			{
				m_position[0] = 0.f; m_position[1] = 0.f; m_position[2] = 0.f;
				m_scale[0] = 1.f; m_scale[1] = 1.f; m_scale[2] = 1.f;
				for (size_t i = 0; i < 9; ++i)
					m_matrix[i] = i/3==i%3 ? 1.f : 0.f;
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
						Matrix3(cmd->m_matrix));

				p_renderer->ModifyCurrentMatrix(transformation);
			}

			void Transform::PopTransformation(const void* ip_data)
			{
				auto p_renderer = Core::GetRenderer();
				p_renderer->PopMatrix();
			}


		} // Commands
	} // Render
} // SDK