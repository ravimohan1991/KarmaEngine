#pragma once

#include "Karma/Core.h"
#include "Karma/Renderer/Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Karma
{
	class KARMA_API SceneModel
	{
	public:
		SceneModel(const std::string& filePath, MeshType mType = MeshType::Mesh);

		void ProcessNode(aiNode* nodeToProcess, const aiScene* theScene);
		std::shared_ptr<Mesh> ProcessMesh(aiMesh* meshToProcess, const aiScene* theScene);

	private:
		std::list<std::shared_ptr<Mesh>> m_Meshes;
	};
}