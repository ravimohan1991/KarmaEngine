/**
 * @file Texture.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the SkeletalMesh class.
 * @version 1.0
 * @date April 16, 2022
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

namespace Karma
{
	/**
	 * @brief Forward declaration
	 *
	 * @todo Think what this declaration is needed in the first place.
	 */
	class VulkanTexture;

	/**
	 * @brief An enum for classifying textures supported and used by the Engine
	 *
	 * @see \ref Karma::Texture class for definition of texture
	 */
	enum class TextureType
	{
		/** A meta placeholder for texture to be supported and used in future*/
		None = 0,
		/** Color map */
		Image,
		/** Probably information of how light bounces from vertex (intensity)*/
		LightMap,
		/** Map of diffuse surface color at each texel on a surface and acts like a decal or paint job on the surface*/
		DiffusionMap
	};

	/**
	 * @brief Texture class
	 *
	 * @note A texture often contains color information and is usually projected onto the triangles of a mesh. In this case, it acts a bit like those silly fake tattoos we used to apply to our arms when we were kids. But a texture can contain other kinds of visual surface properties as well as colors. And a texture needn’t be projected onto a mesh—for example, a texture might be used as a stand-alone data table. The individual picture elements of a texture are called texels to differentiate them from the pixels on the screen.
	 * @todo Need to integrate VulkanTexture in abstract way (vulkantexture shouldn't be explicit in this class, violation of SRP?)
	 */
	class KARMA_API Texture
	{
	public:
		/**
		 * @brief A constructor
		 *
		 * @since Karma 1.0.0
		 */
		Texture();

		/**
		 * @brief Another constructor
		 *
		 * @param tType							The type of Texture
		 * @param filename						The path of the file (including name), relative to the running directory of Engine
		 * @param textureName					Name of texture for identification
		 * @param textureShaderName				Name of the shader with which shader binding is done (for instance, see OpenGLShader::Bind)
		 * @since Karma 1.0.0
		 */
		Texture(TextureType tType, const char* filename, std::string textureName, std::string textureShaderName);

		/**
		 * @brief A destructor
		 *
		 * @since Karma 1.0.0
		 */
		~Texture();

		// Getters
		/**
		 * @brief Getter for texture name
		 *
		 * @since Karma 1.0.0
		 */
		const std::string& GetTextureName() const { return m_TName; }

		/**
		 * @brief Getter for Texture Shader name
		 *
		 * @since Karma 1.0.0
		 */
		const std::string& GetTextureShaderName() const { return m_TShaderName; }

		/**
		 * @brief Getter for VulkanTexture
		 *
		 * @todo Needs to be abstracted (no rendering API specific functionality in this class)
		 * @since Karma 1.0.0
		 */
		std::shared_ptr<VulkanTexture> GetVulkanTexture() const { return m_VulkanTexture; }

	private:
		TextureType m_TType;
		std::string m_TName;

		// Name to be used for identification in the shaders
		std::string m_TShaderName;

		// For Vulkan specific purposes
		std::shared_ptr<VulkanTexture> m_VulkanTexture;
	};
}
