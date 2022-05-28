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

		float* vertexData = new float[meshToProcess->mNumVertices * (3 + 2)];// vec3d + vec2d (position + texture)
		uint32_t vertexDataSize = sizeof(float) * meshToProcess->mNumVertices * (3 + 2);

		unsigned int indexDataSize = 0;

		for (unsigned int i = 0; i < meshToProcess->mNumFaces; i++)
		{
			indexDataSize += meshToProcess->mFaces[i].mNumIndices;
		}

		KR_CORE_INFO("The total indexDataSize is: {0}", indexDataSize);
		uint32_t* indexData = new uint32_t[indexDataSize];

		uint32_t counter = 0;
		uint32_t indexCounter = 0;

		for (unsigned int i = 0; i < meshToProcess->mNumVertices; i++)
		{
			vertexData[counter++] = meshToProcess->mVertices[i].x;
			vertexData[counter++] = meshToProcess->mVertices[i].y;
			vertexData[counter++] = meshToProcess->mVertices[i].z;
			vertexData[counter++] = meshToProcess->mTextureCoords[0][i].x;
			vertexData[counter++] = 1.0f - meshToProcess->mTextureCoords[0][i].y;
		}

		for (unsigned int i = 0; i < meshToProcess->mNumFaces; i++)
		{
			for (unsigned int j = 0; j < meshToProcess->mFaces[i].mNumIndices; j++)
			{
				indexData[indexCounter++] = meshToProcess->mFaces[i].mIndices[j];
			}
		}

		vBuffer.reset(VertexBuffer::Create(vertexData, vertexDataSize));

		// tentative scope
		{
			Karma::BufferLayout layout = {
				{ Karma::ShaderDataType::Float3, "a_Position" },
				{ Karma::ShaderDataType::Float2, "a_UVs" }
			};

			vBuffer->SetLayout(layout);
		}


		iBuffer.reset(IndexBuffer::Create(indexData, indexDataSize));

		delete[] vertexData;
		delete[] indexData;

		productMesh.reset(new Mesh(vBuffer, iBuffer, "Cigar"));

		return productMesh;
	}
}