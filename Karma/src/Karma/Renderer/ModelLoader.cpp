#include "ModelLoader.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <unordered_map>

namespace Karma
{
	ModelLoader::ModelLoader(const std::string& modelSrc)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		bool result = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelSrc.c_str());
		KR_CORE_ASSERT(result == true, warn + err);
		KR_CORE_WARN(warn);

		uint32_t verticesNumber = 0;
		m_IndexCount = 0;
		
		for (const auto& shape : shapes)
		{
			verticesNumber += uint32_t(shape.mesh.indices.size() * (3 + 2 + 4));
			m_IndexCount += uint32_t(shape.mesh.indices.size());
		}
		m_VertexSize = sizeof(float) * verticesNumber;
		m_VertexData = new float[verticesNumber];
		m_IndexData = new uint32_t[m_IndexCount];
		uint32_t counter = 0;
		uint32_t indexCounter = 0;

		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				m_VertexData[counter++] = attrib.vertices[3 * index.vertex_index + 0];
				m_VertexData[counter++] = attrib.vertices[3 * index.vertex_index + 1];
				m_VertexData[counter++] = attrib.vertices[3 * index.vertex_index + 2];
				m_VertexData[counter++] = 1.0f;
				m_VertexData[counter++] = 1.0f;
				m_VertexData[counter++] = 1.0f;
				m_VertexData[counter++] = 1.0f;
				m_VertexData[counter++] = attrib.texcoords[2 * index.texcoord_index + 0];
				// Vulkan/Opengl setting
				m_VertexData[counter++] = 1.0f - attrib.texcoords[2 * index.texcoord_index + 1];
				m_IndexData[indexCounter++] = indexCounter;
			}
		}
	}

	ModelLoader::~ModelLoader()
	{
		delete[] m_VertexData;
		delete[] m_IndexData;
	}
}