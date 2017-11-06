#include "stdafx.h"

#include "MaterialManager.h"

#include "ShaderSystem.h"

#include "Core.h"
#include "Render/IRenderer.h"
#include "Render/TextureManager.h"
#include "Render/ShaderCompiler.h"
#include "Render/RenderWorld.h"
#include "Render/Commands.h"
#include "Resources/ResourceManager.h"

#include "Resources/ResourceManager.h"

#include "FileSystem/FileSystem.h"

#include "PropertyReader.h"

namespace SDK
{	
	
	namespace Resources
	{
		namespace Serialization
		{
			namespace
			{
				using namespace Render;
				using namespace details;

				ShaderVariableType StringToSVT(const std::string& i_type)
				{
					using SVT = ShaderVariableType;
					if (i_type == "sampler2D")
						return SVT::Sampler2D;
					if (i_type == "float")
						return SVT::Float;
					if (i_type == "vec4")
						return SVT::FloatVec4;
					if (i_type == "vec3")
						return SVT::FloatVec3;
					return SVT::Undefined;
				}

				ValueContainer GetPreprocessedValue(const PropertyElement& i_var_elem, ShaderVariableType i_type)
				{
					ValueContainer container;
					using SVT = ShaderVariableType;
					switch (i_type)
					{
						case SVT::Sampler2D:
							{
								auto p_mgr = Core::GetRenderer()->GetTextureManager();
								const auto p_tex_name = i_var_elem.GetValuePtr<std::string>("value");
								if (p_tex_name == nullptr)
								{
									assert(false);
									return std::move(container);
								}
								auto p_load_manager = Core::GetGlobalObject<Resources::ResourceManager>();
								auto handle = p_load_manager->GetHandleToResource<Render::Texture>(*p_tex_name);
								p_load_manager->Use<Render::Texture>(*p_tex_name);
								constexpr size_t hash = 0;
								container.SetValue<Render::TextureHandle>(hash, handle);
							}
							break;
						case SVT::IntVec2:
						case SVT::IntVec3:
						case SVT::IntVec4:
							{
								const auto p_vec = i_var_elem.GetValuePtr<std::vector<int>>("value");
								if (p_vec == nullptr)
								{
									assert(false);
									break;
								}
								const size_t size = p_vec->size()*sizeof(int);
								container.SetValue(0, (const void*)&(*p_vec)[0], size);
							}
							break;
						case SVT::FloatVec2:
						case SVT::FloatVec3:
						case SVT::FloatVec4:
							{
								const auto p_vec = i_var_elem.GetValuePtr<std::vector<float>>("value");
								if (p_vec == nullptr)
								{
									assert(false);
									break;
								}
								const size_t size = p_vec->size()*sizeof(int);
								container.SetValue(0, (const void*)&(*p_vec)[0], size);
							}
							break;
						case SVT::DoubleVec2:
						case SVT::DoubleVec3:
						case SVT::DoubleVec4:
							{
								const auto p_vec = i_var_elem.GetValuePtr<std::vector<double>>("value");
								if (p_vec == nullptr)
								{
									assert(false);
									break;
								}
								const size_t size = p_vec->size()*sizeof(int);
								container.SetValue(0, (const void*)&(*p_vec)[0], size);
							}
							break;
						default:
							{
								const auto p_any = i_var_elem.GetAnyPtr("value");
								if (p_any == nullptr)
								{
									assert(false);
									break;
								}
								container.SetValue(0, p_any->get_raw_ptr(), p_any->get_size());
							}
							break;
					}
					return std::move(container);
				}

				std::vector<MaterialEntry> ParseShaderElement(const PropertyElement& i_shader_elem)
				{
					std::string shader_name = i_shader_elem.GetValue<std::string>("name");
					const auto p_shader = Core::GetGlobalObject<Render::ShaderSystem>()->Access(shader_name);
					if (p_shader == nullptr)
						return std::vector<MaterialEntry>();

					std::vector<MaterialEntry> shader_properties;
					const auto end = i_shader_elem.end<PropertyElement>();
					for (auto it = i_shader_elem.begin<PropertyElement>(); it != end; ++it)
					{
						auto shader_var_name = it.element_name();
						auto uniform = p_shader->GetUniform(shader_var_name);
						// no such uniform
						if (uniform.location == -1)
						{
							//assert(false);
							continue;
						}

						const PropertyElement& element = *it;
						const std::string* p_type = element.GetValuePtr<std::string>("type");
						if (p_type == nullptr)
						{
							// no type property
							assert(false);
							continue;
						}
						ShaderVariableType type = StringToSVT(*p_type);
						if (type == ShaderVariableType::Undefined)
						{
							// unknown type
							assert(false);
							continue;
						}

						std::string show_name = element.GetValue<std::string>("show_name");
						ValueContainer value = std::move(GetPreprocessedValue(element, type));

						shader_properties.emplace_back(uniform.location, std::move(value), type, show_name);
					}
					return std::move(shader_properties);
				}

				bool ParseMaterial(Material& o_material, const PropertyElement& i_material_elem)
				{
					const PropertyElement* p_shader_elem = i_material_elem.GetValuePtr<PropertyElement>("Shader");
					if (p_shader_elem == nullptr)
						return false;

					o_material.m_name = i_material_elem.GetValue<std::string>("name");
					o_material.m_name_hash = Utilities::hash_function(o_material.m_name);
					std::string shader_name = p_shader_elem->GetValue<std::string>("name");
					ShaderHandle handle = Core::GetGlobalObject<Resources::ResourceManager>()->GetHandleToResource<Render::Shader>(shader_name);

					o_material.m_shader.index = handle.index;
					o_material.m_shader.generation = handle.generation;
					if (o_material.m_shader.index == -1)
						return false;

					o_material.m_entries = std::move(ParseShaderElement(*p_shader_elem));

					return true;
				}

			} // namespace

			template <>
			struct LoaderImpl < Render::Material >
			{
				// TODO: is it possible to catch error like ip_streams[3] in compile time?
				static std::pair<LoadResult, Render::Material> Load(std::istream* ip_streams[1], void*)
				{
					using namespace Render;
					Material material;
					
					PropretyReader<(int)ReaderType::SDKFormat> reader;
					PropertyElement root = reader.Parse(*ip_streams[0]);
					const PropertyElement* p_material_elem = root.GetValuePtr<PropertyElement>("Material");
					if (p_material_elem == nullptr)
						return std::make_pair(LoadResult::Failure, material);
					const bool result = ParseMaterial(material, *p_material_elem);
					return std::make_pair(result ? LoadResult::Success : LoadResult::Failure, std::move(material));
				}

				static InternalHandle CreateNewHandle()
				{
					auto handle = Core::GetGlobalObject<Render::MaterialManager>()->m_materials.CreateNew();
					return{ handle.index, handle.generation };
				}

				static void RemoveHandle(InternalHandle i_handle)
				{
					Core::GetGlobalObject<Render::MaterialManager>()->m_materials.Destroy({ i_handle.index, i_handle.generation });
				}

				static void UnloadResource(InternalHandle i_handle)
				{
					Core::GetGlobalObject<Render::MaterialManager>()->m_materials.Destroy({ i_handle.index, i_handle.generation });
				}

				static void Register(InternalHandle i_handle, Render::Material i_material)
				{
					Core::GetGlobalObject<Render::MaterialManager>()->m_materials.m_elements[i_handle.index].second = std::move(i_material);
				}
			};

		} // Serialization

	} // Resources

	namespace Render
	{
		void MaterialManager::Initialize()
		{
			auto p_load_manager = Core::GetGlobalObject<Resources::ResourceManager>();
			p_load_manager->RegisterLoader<MaterialManager, Material>(*this, &MaterialManager::Load, "material");
		}

		void MaterialManager::Release()
		{
			auto p_load_manager = Core::GetGlobalObject<Resources::ResourceManager>();
			p_load_manager->Unregister<MaterialManager, Material>("material");
		}

		void MaterialManager::Load(const PropertyElement& i_resource_element)
		{
			const std::string resource_name = i_resource_element.GetValue<std::string>("resource_name");
			const std::string path = i_resource_element.GetValue<std::string>("path");
			Load(resource_name, path);
		}

		MaterialHandle MaterialManager::Load(const std::string& i_resource_name, const std::string& i_path)
		{
			auto p_load_manager = Core::GetGlobalObject<Resources::ResourceManager>();
			InternalHandle handle = p_load_manager->Load<Material>(i_resource_name, { i_path }, nullptr);

			// resource is already loaded
			if (handle.index != -1)
			{
				assert(handle.index >= 0 && handle.index < static_cast<int>(m_materials.m_elements.size()));
				return m_materials.m_elements[handle.index].first;
			}

			return MaterialHandle::InvalidHandle();
		}

		const Material* MaterialManager::AccessMaterial(MaterialHandle i_handle) const
		{
			return m_materials.Access(i_handle);
		}

		void MaterialManager::RemoveMaterial(MaterialHandle i_handle)
		{
			auto p_load_manager = Core::GetGlobalObject<Resources::ResourceManager>();
			p_load_manager->Unload<Material>({ i_handle.index, i_handle.generation });			
		}

		void* MaterialManager::SetupShaderAndCreateCommands(RenderCommandBucket& o_bucket, ShaderUniformValue* op_dynamic_unis, size_t& o_setuped_number, size_t i_unis_size, const Material& i_material, void* ip_shader_command) const
		{
			o_setuped_number = 0;
			// bind textures
			void* p_parent = ip_shader_command;
			int target = 0;
			size_t uni_index = 0;
			for (auto& entry : i_material.m_entries)
			{
				if (uni_index >= i_unis_size)
				{
					assert(false && "Unis is not enough");
					return p_parent;
				}

				if (entry.type != ShaderVariableType::Sampler2D)
					continue;

				Commands::BindTexture* p_bind_texture = o_bucket.Append<Commands::BindTexture>(p_parent);
				const InternalHandle* p_handle = reinterpret_cast<const InternalHandle*>(entry.container.GetDataPtr());
				if (p_handle == nullptr)
				{
					assert(false && "Material manager: Null handle to texture");
					continue;
				}
				
				p_bind_texture->texture_handle = *p_handle;
				p_bind_texture->target = target;
				op_dynamic_unis[uni_index] = ShaderUniformValue::Construct(entry.shader_var_location, entry.type, &target, sizeof(int), false);

				// increase target and uni_index
				++target;
				++uni_index;
				++o_setuped_number;
				// newly created command will be parent to next
				p_parent = p_bind_texture;
			}

			return p_parent;
		}

	} // Render
} // SDK