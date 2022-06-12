#pragma once

#include "Karma/Core.h"
#include "Karma/Renderer/Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// PCH stuff
#include <memory>
#include "Karma/Log.h"

namespace Karma
{
	class KARMA_API SceneModel
	{
	public:
		SceneModel(const std::string& filePath, MeshType mType = MeshType::Mesh);

		void ProcessNode(aiNode* nodeToProcess, const aiScene* theScene);

		const std::vector<std::shared_ptr<Mesh>>&  GetMeshList() const { return m_Meshes; }

	private:
		std::vector<std::shared_ptr<Mesh>> m_Meshes;
	};
}
