#include "stdafx.h"

#include "ResourceManager.h"
#include "Core.h"
#include "Render/IRenderer.h"
// Loaders
#include "Render/MaterialManager.h"
#include "Render/ShaderSystem.h"
#include "Render/TextureManager.h"
#include "Render/MeshSystem.h"
#include "Render/Fonts/FontManager.h"

namespace SDK
{
	namespace Resources
	{

		void RegisterLoaders()
		{
			Core::GetGlobalObject<Render::MaterialManager>()->Initialize();
			Core::GetGlobalObject<Render::ShaderSystem>()->Initialize();
			Core::GetGlobalObject<Render::MeshSystem>()->Initialize();
			Core::GetGlobalObject<Render::FontManager>()->Initialize();
			Core::GetRenderer()->GetTextureManager()->Initialize();
		}

		void ReleaseLoaders()
		{
			Core::GetGlobalObject<Render::MaterialManager>()->Release();
			Core::GetGlobalObject<Render::ShaderSystem>()->Release();
			Core::GetGlobalObject<Render::MeshSystem>()->Release();
			Core::GetGlobalObject<Render::FontManager>()->Release();
			Core::GetRenderer()->GetTextureManager()->Release();
		}

	} // Resources
} // SDK