#include "SceneModel.h"

namespace Karma
{
	SceneModel::SceneModel(const std::string& filePath, MeshType mType)
	{
		Assimp::Importer assImporter;

		const aiScene* scene = assImporter.ReadFile(filePath, aiProcess_Triangulate);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
            KR_CORE_ERROR("ERROR ASSIMP:: {0}", assImporter.GetErrorString());
		}

		ProcessNode(scene->mRootNode, scene);
	}

	void SceneModel::ProcessNode(aiNode* nodeToProcess, const aiScene* theScene)
	{
		for (unsigned int i = 0; i < nodeToProcess->mNumMeshes; i++)
		{
			aiMesh* mesh = theScene->mMeshes[nodeToProcess->mMeshes[i]];
			
			m_Meshes.push_back(Mesh::ProcessTheRawMesh(mesh));
		}
		
		for (unsigned int i = 0; i < nodeToProcess->mNumChildren; i++)
		{
			ProcessNode(nodeToProcess->mChildren[i], theScene);
		}
	}
}
