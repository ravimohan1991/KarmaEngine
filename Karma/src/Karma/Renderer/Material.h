/**
 * @file Material.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the Material class.
 * @version 1.0
 * @date April 16, 2022
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

#include "Karma/Renderer/Shader.h"

#include "Texture.h"
#include "Camera/Camera.h"

namespace Karma
{
	/**
	 * @brief A class for shader + texture
	 *
	 * A material is a complete description of the visual properties of a mesh. This includes a specification of the textures that are mapped to its surface and also various higher-level properties, such as which shader programs to use when rendering the mesh, the input parameters to those shaders, and other parameters that control the functionality of the graphics acceleration hardware itself.
	 *
	 * @note Mesh-material pair contains all the information we need to render the object
	 */
	class KARMA_API Material
	{
	public:
		/**
		 * @brief A constructor
		 *
		 * @since Karma 1.0.0
		 */
		Material();

		/**
		 * @brief A hook into the game loop for updating uniforms and related stuff
		 *
		 * @since Karma 1.0.0
		 */
		void OnUpdate();

		/**
		 * @brief Process for various things belonging to the material before submitting the VertexArray. Includes uploading uniforms to GPU.
		 *
		 * @see VertexArray::UpdateProcessAndSetReadyForSubmission()
		 * @since Karma 1.0.0
		 */
		void ProcessForSubmission();

		/**
		 * @brief Caching the current camera (m_MainCamera)
		 *
		 * @since Karma 1.0.0
		 */
		void AttatchMainCamera(std::shared_ptr<Camera> mCamera);

		/**
		 * @brief Add to the list of shaders used by this material
		 *
		 * @since Karma 1.0.0
		 */
		void AddShader(std::shared_ptr<Shader> shader) { m_Shaders.push_back(shader); }

		/**
		 * @brief Remove from the list of shaders
		 *
		 * @since Karma 1.0.0
		 */
		void RemoveShader(std::shared_ptr<Shader> shader) { m_Shaders.remove(shader); }

		/**
		 * @brief Add to the list of textures used by this material
		 *
		 * @since Karma 1.0.0
		 */
		void AddTexture(std::shared_ptr<Texture> texture) { m_Textures.push_back(texture); }

		/**
		 * @brief Remove from the list of textures
		 *
		 * @since Karma 1.0.0
		 */
		void RemoveTexture(std::shared_ptr<Texture> texture) { m_Textures.remove(texture); }

		// Getters
		/**
		 * @brief Getter for shader list
		 *
		 * @since Karma 1.0.0
		 */
		std::shared_ptr<Shader> GetShader(const std::string& shaderName) const;

		/**
		 * @brief Getter for shader by index
		 *
		 * @since Karma 1.0.0
		 */
		std::shared_ptr<Shader> GetShader(int index);

		/**
		 * @brief Getter for texture by index
		 *
		 * @since Karma 1.0.0
		 */
		std::shared_ptr<Texture> GetTexture(int index);

		// May add Physics-relevant features in future.

	private:
		// References to shaders, textures
		std::list<std::shared_ptr<Shader>> m_Shaders;
		std::list<std::shared_ptr<Texture>> m_Textures;

		std::shared_ptr<Camera> m_MainCamera;
	};
}
