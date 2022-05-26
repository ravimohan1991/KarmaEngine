#include "Mesh.h"

namespace Karma
{
	Mesh::Mesh(std::shared_ptr<VertexBuffer> vertexBuffer, std::shared_ptr<IndexBuffer> indexBuffer, const std::string& meshName)
	{
		if (vertexBuffer)
		{
			m_VertexBuffer = vertexBuffer;
		}
		else
		{
			m_VertexBuffer = nullptr;
		}

		if (indexBuffer)
		{
			m_IndexBuffer = indexBuffer;
		}
		else
		{
			m_VertexBuffer = nullptr;
		}

		m_MeshName = meshName;
	}

	void Mesh::ProcessMesh()
	{
		
	}
}