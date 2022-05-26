#pragma once

#include "Karma/Core.h"
#include "Mesh.h"

namespace Karma
{
	struct BoneInfo
	{
		/* The index in u_FinalBonesMatrices. */
		int m_Id;

		/* The matrix transformation of vertices from model space to bone space. */
		glm::mat4 m_ModelToBoneTransform; // corresponds to u_FinalBonesMatrices in the vertex shader
	};

	class KARMA_API SkeletalMesh : public Mesh
	{
	public:
		SkeletalMesh(std::shared_ptr<VertexBuffer> vertexBuffer, std::shared_ptr<IndexBuffer> indexBuffer, const std::string& meshName = "NoName");

		virtual void ProcessMesh() override;
	};
}