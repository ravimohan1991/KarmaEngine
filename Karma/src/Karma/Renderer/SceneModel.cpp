#include "SceneModel.h"

namespace Karma
{
	SceneModel::SceneModel(const std::string& filePath, MeshType mType)
	{
		Assimp::Importer assImporter;

		const aiScene* scene = assImporter.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs);

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
			
			m_Meshes.push_back(ProcessMesh(mesh, theScene));
		}
		
		for (unsigned int i = 0; i < nodeToProcess->mNumChildren; i++)
		{
			ProcessNode(nodeToProcess->mChildren[i], theScene);
		}
	}

	std::shared_ptr<Mesh> SceneModel::ProcessMesh(aiMesh* meshToProcess, const aiScene* theScene)
	{
		std::shared_ptr<Mesh> productMesh;

		std::shared_ptr<VertexBuffer> vBuffer;
		std::shared_ptr<IndexBuffer> iBuffer;

		//vBuffer.reset(VertexBuffer::Create());
		KR_CORE_INFO("Processing the mesh with bone status {0}", meshToProcess->HasBones());
		
		/*
		productMesh.reset(new Mesh(vertexbuffer, indexbuffer, "name"));
		
		vector<Vertex> vertices;
		vector<unsigned int> indices;
		vector<Texture> textures;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			// process vertex positions, normals and texture coordinates
			[...]
			vertices.push_back(vertex);
		}
		// process indices
		[...]
		// process material
		if (mesh->mMaterialIndex >= 0)
		{
			[...]
		}

		return Mesh(vertices, indices, textures);
		*/
		return productMesh;
	}
}