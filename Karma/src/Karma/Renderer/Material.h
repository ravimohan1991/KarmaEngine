#pragma once

#include "Karma/Core.h"
#include "Karma/Renderer/Shader.h"
#include "Texture.h"
#include "Camera/Camera.h"

// PCH stuff
#include <list>
#include "Karma/Log.h"

namespace Karma
{
	class KARMA_API Material
	{
	public:
		Material();

		void OnUpdate();
		void ProcessForSubmission();

		void AttatchMainCamera(std::shared_ptr<Camera> mCamera);
		void AddShader(std::shared_ptr<Shader> shader) { m_Shaders.push_back(shader); }
		void RemoveShader(std::shared_ptr<Shader> shader) { m_Shaders.remove(shader); }
		void AddTexture(std::shared_ptr<Texture> texture) { m_Textures.push_back(texture); }
		void RemoveTexture(std::shared_ptr<Texture> texture) { m_Textures.remove(texture); }

		// Getters
		std::shared_ptr<Shader> GetShader(const std::string& shaderName) const;

		// May add Physics-relevant features in future.

	private:
		// References to shaders, textures
		std::list<std::shared_ptr<Shader>> m_Shaders;
		std::list<std::shared_ptr<Texture>> m_Textures;

		std::shared_ptr<Camera> m_MainCamera;
	};
}
