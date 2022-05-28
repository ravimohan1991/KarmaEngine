#pragma once

#include "Karma/Core.h"
#include "Buffer.h"

namespace Karma
{	
	enum class MeshType
	{
		Mesh = 0,
		SkeletalMesh
	};
	
	// Wrapper for VertexBuffer + IndexBuffer
	class KARMA_API Mesh
	{
	public:
		Mesh(std::shared_ptr<VertexBuffer> vertexBuffer, std::shared_ptr<IndexBuffer> indexBuffer, const std::string& meshName = "NoName");

		virtual void ProcessMesh();

		std::shared_ptr<VertexBuffer> GetVertexBuffer() const { return m_VertexBuffer; }
		std::shared_ptr<IndexBuffer> GetIndexBuffer() const { return m_IndexBuffer; }

		void SetVertexBuffer(std::shared_ptr<VertexBuffer> vBuffer) { m_VertexBuffer = vBuffer; }
		void SetIndexBuffer(std::shared_ptr<IndexBuffer> iBuffer) { m_IndexBuffer = iBuffer; }

	protected:
		std::shared_ptr<VertexBuffer> m_VertexBuffer;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;

		std::string m_MeshName;
	};
}