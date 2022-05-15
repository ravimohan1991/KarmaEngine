#pragma once

#include "Karma/Core.h"
#include "Buffer.h"

namespace Karma
{
	// Wrapper for VertexBuffer + IndexBuffer
	class KARMA_API Mesh
	{
	public:
		Mesh(std::shared_ptr<VertexBuffer> vertexBuffer, std::shared_ptr<IndexBuffer> indexBuffer, const std::string& meshName = "NoName");

		std::shared_ptr<VertexBuffer> GetVertexBuffer() { return m_VertexBuffer; }
		std::shared_ptr<IndexBuffer> GetIndexBuffer() { return m_IndexBuffer; }

	private:
		std::shared_ptr<VertexBuffer> m_VertexBuffer;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;

		std::string m_MeshName;
	};
}