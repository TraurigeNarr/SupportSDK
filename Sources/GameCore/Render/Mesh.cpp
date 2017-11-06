#include "stdafx.h"

#include "Mesh.h"

#include "Core.h"
#include "HardwareBufferManagerBase.h"
#include "IRenderer.h"

namespace SDK
{

	namespace Render
	{
		Mesh::Mesh()
			: m_name("InvalidMesh")
			, m_name_hash(Utilities::hash_function(std::string("InvalidMesh")))
		{}

		Mesh::Mesh(const std::string& i_name)
			: m_name(i_name)
			, m_name_hash(Utilities::hash_function(i_name))
		{
		}

		Mesh::Mesh(const Mesh& i_other)
			: m_name(i_other.m_name)
			, m_name_hash(i_other.m_name_hash)
		{
			for (size_t i = 0; i < i_other.GetSubmeshNumber(); ++i)
			{
				const Mesh::SubMesh& sub_mesh = i_other.m_sub_meshes[i];
				AddSubmesh(
					sub_mesh.m_name,
					sub_mesh.m_vertex_buffer,
					sub_mesh.m_pos_layout,
					sub_mesh.m_normal_layout,
					sub_mesh.m_uv_layout,
					sub_mesh.m_index_buffer,
					sub_mesh.m_vertices_count,
					sub_mesh.m_triangles_count);
				for (size_t material_i = 0; material_i < sub_mesh.m_materials.size(); ++material_i)
				{
					m_sub_meshes[i].m_materials.push_back(sub_mesh.m_materials[material_i]);
				}
			}
		}

		void Mesh::AddSubmesh(
			const std::string& i_name,
			VertexBufferHandle i_vertices,
			VertexLayoutHandle i_pos_layout,
			VertexLayoutHandle i_normal_layout,
			VertexLayoutHandle i_uv_layout,
			IndexBufferHandle i_indices,
			size_t i_vertices_size,
			size_t i_triangles_size)
		{
			m_sub_meshes.emplace_back(i_name, i_vertices, i_pos_layout, i_normal_layout, i_uv_layout, i_indices, i_vertices_size, i_triangles_size);
		}

	} // Render

} // SDK