/**
 * @file Mesh.h
 * @author Ravi Mohan (the_cowboy)
 * @brief This file contains the class Mesh and relevant data structures.
 * @version 1.0
 * @date May 15, 2022
 *
 * @copyright Karma Engine copyright(c) People of India
 */

#pragma once

#include "krpch.h"

#include "Buffer.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Karma
{	
	enum class MeshType
	{
		Mesh = 0,
		SkeletalMesh
	};

	/**
	 * @brief An enum for cherning data by interleaving the following attributes
	 *
	 * @since Karma 1.0.0
	 */
	enum class MeshAttribute
	{
		/** Vertex attribute */
		Vertices = 0,
		/** Normal attribute */
		Normals,
		/** Tangent attribute */
		Tangents,
		/** Bitangents attribute  which IDK */
		Bitangents,
		/** Color attribute */
		Colors,
		/** Texture coordinates attributes */
		TextureCoords,
		/** Animation mesh attribute */
		AnimMeshes
	};
	
	/**
	 * @brief An organized collection of vertex and index buffers along with rest of the model specific information which includes colors, texture coordinates
	 * and perhaps animation attributes.
	 */
	class KARMA_API Mesh
	{
	public:
		Mesh(std::shared_ptr<VertexBuffer> vertexBuffer, std::shared_ptr<IndexBuffer> indexBuffer, const std::string& meshName = "NoName",
			MeshType mType = MeshType::Mesh);
		Mesh(const std::string& filePath);

		virtual void ProcessMesh(aiMesh* meshToProcess);

		/**
		 * @brief Generate a layout (\ref BufferLayout) for the supplied mesh
		 *
		 * @param meshToProcess				The Assimp mesh whose layout is to be generated
		 * @param mName						Mesh name
		 *
		 * @since Karma 1.0.0
		 */
		static std::shared_ptr<Mesh> ProcessTheRawMesh(aiMesh* meshToProcess, const std::string& mName = "NoName");
		static void DealVertexIndexBufferData(float*& vertexData, uint32_t& vertexDataSize, uint32_t*& indexData, uint32_t& indexDataLength,
			aiMesh* meshToProcess, BufferLayout& buffLayout);

		void ProcessNode(aiNode* nodeToProcess, const aiScene* theScene);

		static void GaugeVertexDataLayout(aiMesh* meshToProcess, BufferLayout& buffLayout);

		std::shared_ptr<VertexBuffer> GetVertexBuffer() const { return m_VertexBuffer; }
		std::shared_ptr<IndexBuffer> GetIndexBuffer() const { return m_IndexBuffer; }

		void SetVertexBuffer(std::shared_ptr<VertexBuffer> vBuffer) { m_VertexBuffer = vBuffer; }
		void SetIndexBuffer(std::shared_ptr<IndexBuffer> iBuffer) { m_IndexBuffer = iBuffer; }

		// Useful dictionary related functions
		static float LayoutElementToAttributeValue(unsigned int vertexNumber, uint32_t counter, aiMesh* meshToProcess, const BufferElement& layoutElem);
		static void InitializeAttributeDictionary();

	protected:
		std::shared_ptr<VertexBuffer> m_VertexBuffer;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;

		std::string m_MeshName;
		MeshType m_MeshType;

		static std::shared_ptr<std::unordered_map<std::string, MeshAttribute>> m_NameToAttributeDictionary;
	};
}
