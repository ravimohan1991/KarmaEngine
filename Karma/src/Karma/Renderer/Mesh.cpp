#include "Mesh.h"

namespace Karma
{
	// We initialize the dictionary
	// Make sure to add elements to dictionary for vertex attribute extension
	std::shared_ptr<std::unordered_map<std::string, MeshAttribute>> Mesh::m_NameToAttributeDictionary = std::make_shared<std::unordered_map<std::string, MeshAttribute>>();

	Mesh::Mesh(std::shared_ptr<VertexBuffer> vertexBuffer, std::shared_ptr<IndexBuffer> indexBuffer, const std::string& meshName, 
		MeshType mType)
	{
		InitializeAttributeDictionary();

		if (vertexBuffer)
		{
			m_VertexBuffer = vertexBuffer;
		}
		else
		{
			m_VertexBuffer = nullptr;
			KR_CORE_WARN("Mesh {0} is being initialized with no Vertex Buffer", meshName);
		}

		if (indexBuffer)
		{
			m_IndexBuffer = indexBuffer;
		}
		else
		{
			m_VertexBuffer = nullptr;
			KR_CORE_WARN("Mesh {0} is being initialized with no Index Buffer", meshName);
		}

		m_MeshName = meshName;
		m_MeshType = mType;
	}

	Mesh::Mesh(const std::string& filePath)
	{
		InitializeAttributeDictionary();
		
		Assimp::Importer assImporter;

		const aiScene* scene = assImporter.ReadFile(filePath, aiProcess_Triangulate);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			KR_CORE_ERROR("ERROR ASSIMP:: {0}", assImporter.GetErrorString());
		}

		ProcessNode(scene->mRootNode, scene);
	}

	void Mesh::ProcessNode(aiNode* nodeToProcess, const aiScene* theScene)
	{
		for (unsigned int i = 0; i < nodeToProcess->mNumMeshes; i++)
		{
			aiMesh* mesh = theScene->mMeshes[nodeToProcess->mMeshes[i]];

			ProcessMesh(mesh);

			// We shall halt after first success identification of full fledged mesh!
			// We need to provide the ability for User to choose/single out a mesh (in a file of many)
			return;
		}

		for (unsigned int i = 0; i < nodeToProcess->mNumChildren; i++)
		{
			ProcessNode(nodeToProcess->mChildren[i], theScene);
		}
	}

	std::shared_ptr<Mesh> Mesh::ProcessTheRawMesh(aiMesh* meshToProcess, const std::string& mName)
	{
		InitializeAttributeDictionary();
		
		std::shared_ptr<Mesh> productMesh;
		std::shared_ptr<VertexBuffer> vBuffer;
		std::shared_ptr<IndexBuffer> iBuffer;

		float* vertexData;
		uint32_t vertexDataSize, indexDataLength;
		uint32_t* indexData;
		BufferLayout layout;

		DealVertexIndexBufferData(vertexData, vertexDataSize, indexData, indexDataLength, meshToProcess, layout);

		vBuffer.reset(VertexBuffer::Create(vertexData, vertexDataSize));
		vBuffer->SetLayout(layout);

		iBuffer.reset(IndexBuffer::Create(indexData, indexDataLength));

		delete[] vertexData;
		delete[] indexData;

		productMesh.reset(new Mesh(vBuffer, iBuffer, mName));

		return productMesh;
	}

	void Mesh::ProcessMesh(aiMesh* meshToProcess)
	{
		float* vertexData;
		uint32_t vertexDataSize, indexDataLength;
		uint32_t* indexData;
		BufferLayout layout;

		DealVertexIndexBufferData(vertexData, vertexDataSize, indexData, indexDataLength, meshToProcess, layout);

		m_VertexBuffer.reset(VertexBuffer::Create(vertexData, vertexDataSize));
		m_VertexBuffer->SetLayout(layout);

		m_IndexBuffer.reset(IndexBuffer::Create(indexData, indexDataLength));

		delete[] vertexData;
		delete[] indexData;
	}

	void Mesh::DealVertexIndexBufferData(float*& vertexData, uint32_t& vertexDataSize, uint32_t*& indexData, uint32_t& indexDataLength,
		aiMesh* meshToProcess, BufferLayout& buffLayout)
	{
		GaugeVertexDataLayout(meshToProcess, buffLayout);

		uint32_t layoutSlots = 0;

		for (const auto& elem : buffLayout.GetElements())
		{
			layoutSlots += elem.GetComponentCount();
		}
		
		uint32_t vertexDataLength = meshToProcess->mNumVertices * layoutSlots;
		
		vertexData = new float[vertexDataLength];
		vertexDataSize = sizeof(float) * vertexDataLength;

		indexDataLength = 0;

		for (unsigned int i = 0; i < meshToProcess->mNumFaces; i++)
		{
			indexDataLength += meshToProcess->mFaces[i].mNumIndices;
		}

		indexData = new uint32_t[indexDataLength];

		uint32_t counter = 0;

		// We are assuming that primitive types are only triangles
		// See aiMesh structure for relevant information
		for (unsigned int i = 0; i < meshToProcess->mNumVertices; i++)
		{
			// Now, based on the gauged layout, we interleave all of the
			// attributes		
			for (const auto& layoutElem : buffLayout.GetElements())
			{
				for (uint32_t j = 0; j < layoutElem.GetComponentCount(); j++)
				{
					vertexData[counter++] = LayoutElementToAttributeValue(i, j, meshToProcess, layoutElem);
				}
			}
			
		}

		counter = 0;

		for (unsigned int i = 0; i < meshToProcess->mNumFaces; i++)
		{
			for (unsigned int j = 0; j < meshToProcess->mFaces[i].mNumIndices; j++)
			{
				indexData[counter++] = meshToProcess->mFaces[i].mIndices[j];
			}
		}
	}

	void Mesh::GaugeVertexDataLayout(aiMesh* meshToProcess, BufferLayout& buffLayout)
	{
		buffLayout.PushElement({ ShaderDataType::Float3, "v_Position" });

		if (meshToProcess->mTextureCoords != nullptr)
		{
			buffLayout.PushElement({ ShaderDataType::Float2, "v_UV" });
		}
		if (meshToProcess->mColors[0] != nullptr)
		{
			buffLayout.PushElement({ ShaderDataType::Float4, "v_Color" });
		}
		if (meshToProcess->mNormals != nullptr)
		{
			buffLayout.PushElement({ ShaderDataType::Float3, "v_Normal" });
		}
		if (meshToProcess->mTangents != nullptr)
		{
			buffLayout.PushElement({ ShaderDataType::Float3, "v_Tangent" });
		}
	}

	// Helpers

	// May need optimization, but this is used only during loading of mesh.  So ponder ...
	float Mesh::LayoutElementToAttributeValue(unsigned int vertexNumber, uint32_t counter, aiMesh* meshToProcess, const BufferElement& layoutElem)
	{
		switch ((*m_NameToAttributeDictionary)[layoutElem.Name])
		{
			// Vertex coordinates interleaving
			case MeshAttribute::Vertices:
					{
						if (counter == 0)
						{
							return meshToProcess->mVertices[vertexNumber].x;
						}
						else if (counter == 1)
						{
							return meshToProcess->mVertices[vertexNumber].y;
						}
						else if (counter == 2)
						{
							return meshToProcess->mVertices[vertexNumber].z;
						}
					}
				break;
			
			// We assume that AI_MAX_NUMBER_OF_TEXTURECOORDS = 1.  This simplifies but may have
			// some future extention IF we delve deep into Textures.
			// For now we interleave the UV coordinates 
			case MeshAttribute::TextureCoords:
					{
						if (counter == 0)
						{
							return meshToProcess->mTextureCoords[0][vertexNumber].x;
						}
						else if (counter == 1)
						{
							return meshToProcess->mTextureCoords[0][vertexNumber].y;
						}
					}
				break;

			case MeshAttribute::Normals:
					{
						if (counter == 0)
						{
							return meshToProcess->mNormals[vertexNumber].x;
						}
						else if (counter == 1)
						{
							return meshToProcess->mNormals[vertexNumber].y;
						}
						else if (counter == 2)
						{
							return meshToProcess->mNormals[vertexNumber].z;
						}
					}
				break;

			case MeshAttribute::Colors:
					{
						if (counter == 0)
						{
							return meshToProcess->mColors[0][vertexNumber].r;
						}
						else if (counter == 1)
						{
							return meshToProcess->mColors[0][vertexNumber].g;
						}
						else if (counter == 2)
						{
							return meshToProcess->mColors[0][vertexNumber].b;
						}
						else if(counter == 3)
						{
							return meshToProcess->mColors[0][vertexNumber].a;
						}
					}
				break;

			case MeshAttribute::Tangents:
					{
						if (counter == 0)
						{
							return meshToProcess->mTangents[vertexNumber].x;
						}
						else if (counter == 1)
						{
							return meshToProcess->mTangents[vertexNumber].y;
						}
						else if (counter == 2)
						{
							return meshToProcess->mTangents[vertexNumber].z;
						}
					}
				break;

			default:
				KR_CORE_INFO("Interleaving unknown attribute {0} for vertex {1}", layoutElem.Name, vertexNumber);
				return 101.0f;
				break;
		}
		
		KR_CORE_WARN("Outside the scope of Switch.  This should never happen!  Check the leakages.");
		return 101.0f;
	}

	void Mesh::InitializeAttributeDictionary()
	{
		if (m_NameToAttributeDictionary->empty())
		{
			m_NameToAttributeDictionary->insert({ "v_UV", MeshAttribute::TextureCoords });
			m_NameToAttributeDictionary->insert({ "v_Color", MeshAttribute::Colors });
			m_NameToAttributeDictionary->insert({ "v_Normal", MeshAttribute::Normals });
			m_NameToAttributeDictionary->insert({ "v_Tangent", MeshAttribute::Tangents });
		}
	}
}