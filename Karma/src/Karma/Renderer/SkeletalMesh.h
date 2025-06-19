/**
 * @file SkeletalMesh.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the SkeletalMesh class.
 * @version 1.0
 * @date May 26, 2022
 *
 * @copyright Karma Engine copyright(c) People of India
 */
#pragma once

#include "krpch.h"

#include "Mesh.h"

namespace Karma
{
	/**
	 * @brief A structure of bone information used to identify in SkeletalMesh
	 */
	struct BoneInfo
	{
		/** 
		 * @brief The index in u_FinalBonesMatrices.
		 */
		int m_Id;

		/**
		 * @brief The matrix transformation of vertices from model space to bone space.
		 */
		glm::mat4 m_ModelToBoneTransform; // corresponds to u_FinalBonesMatrices in the vertex shader
	};

	/**
	 * @brief SkeletalMesh class
	 */
	class KARMA_API SkeletalMesh : public Mesh
	{
	public:
		/**
		 * @brief A constructor
		 *
		 * @param vertexBuffer				Path to vertex shader (filename included). For instance "../Resources/Shaders/shader.vert", relative to
		 * 									Engine's running directory
		 * @param indexBuffer				Path to indexBuffer
		 *
		 * @param meshName					Name of the mesh (m_MeshName)
		 * @since Karma 1.0.0
		 */
		SkeletalMesh(std::shared_ptr<VertexBuffer> vertexBuffer, std::shared_ptr<IndexBuffer> indexBuffer, const std::string& meshName = "NoName");

		/**
		 * @brief To be written appropriately
		 *
		 * @since Karma 1.0.0
		 */
		virtual void ProcessMesh(aiMesh* meshToProcess) override;
	};
}
