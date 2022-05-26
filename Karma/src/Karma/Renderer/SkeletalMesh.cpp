#include "SkeletalMesh.h"

namespace Karma
{
	SkeletalMesh::SkeletalMesh(std::shared_ptr<VertexBuffer> vertexBuffer, std::shared_ptr<IndexBuffer> indexBuffer, const std::string& meshName) :
		Mesh(vertexBuffer, indexBuffer, meshName)
	{
	}

	void SkeletalMesh::ProcessMesh()
	{

	}
}