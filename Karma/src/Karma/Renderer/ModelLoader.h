#pragma once
#include "Karma/Core.h"

namespace Karma
{
	class KARMA_API ModelLoader
	{
	public:
		ModelLoader(const std::string& modelSrc);
		~ModelLoader();

		float* GetVertexData() const { return m_VertexData; }
		uint32_t GetVertexSize() const { return m_VertexSize; }
		uint32_t* GetIndexData() const { return m_IndexData; }
		uint32_t GetIndexCount() const { return m_IndexCount; }
	private:
		float* m_VertexData;
		uint32_t* m_IndexData;
		uint32_t m_VertexSize;
		uint32_t m_IndexCount;
	};
}