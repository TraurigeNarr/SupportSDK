#ifndef __GAMECORE_MESH_H__
#define __GAMECORE_MESH_H__

#include "../GameCoreAPI.h"

#include "RenderTypes.h"

#include <Utilities/HashFunctions.h>

namespace SDK
{
	namespace Render
	{
		class MeshSystem;
		
		class Mesh
		{
		public:
			struct SubMesh
			{
				size_t m_name_hash;
				std::string m_name;
				VertexBufferHandle	m_vertex_buffer;
				VertexLayoutHandle	m_pos_layout;
				VertexLayoutHandle	m_normal_layout;
				VertexLayoutHandle	m_uv_layout;

				IndexBufferHandle	m_index_buffer;

				std::vector<MaterialHandle> m_materials;

				size_t m_vertices_count;
				size_t m_triangles_count;

				SubMesh(
					const std::string& i_name,
					VertexBufferHandle i_vertices,
					VertexLayoutHandle i_pos_layout,
					VertexLayoutHandle i_normal_layout,
					VertexLayoutHandle i_uv_layout,
					IndexBufferHandle i_indices,
					size_t i_vertices_count,
					size_t i_triangles_count)
					: m_name(i_name)
					, m_name_hash(Utilities::hash_function(i_name))
					, m_vertex_buffer(i_vertices)
					, m_pos_layout(i_pos_layout)
					, m_normal_layout(i_normal_layout)
					, m_uv_layout(i_uv_layout)
					, m_index_buffer(i_indices)
					, m_vertices_count(i_vertices_count)
					, m_triangles_count(i_triangles_count)
				{}
			};

		private:
			std::vector<SubMesh> m_sub_meshes;
			size_t m_name_hash;
			std::string m_name;

		public:
			GAMECORE_EXPORT Mesh();
			GAMECORE_EXPORT Mesh(const std::string& i_name);
			GAMECORE_EXPORT Mesh(const Mesh& i_other);

			GAMECORE_EXPORT void AddSubmesh(
					const std::string& i_name,
					VertexBufferHandle i_vertices,
					VertexLayoutHandle i_pos_layout,
					VertexLayoutHandle i_normal_layout,
					VertexLayoutHandle i_uv_layout,
					IndexBufferHandle i_indices,
					size_t i_vertices_size,
					size_t i_triangles_size);

			size_t GetSubmeshNumber() const { return m_sub_meshes.size(); }
			const SubMesh& GetSubmesh(size_t i) const { return m_sub_meshes[i]; }
			SubMesh& GetSubmesh(size_t i) { return m_sub_meshes[i]; }
			bool IsValid() const { return !m_sub_meshes.empty(); }

			std::string GetName() const {
				return m_name;
			}

			size_t GetNameHash() const {
				return m_name_hash;
			}
		};

	} // Render
} // SDK

#endif