#include "OpenGLVertexArray.h"
#include "glad/glad.h"

namespace Karma
{
	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
		case Karma::ShaderDataType::Float:
			return GL_FLOAT;
		case Karma::ShaderDataType::Float2:
			return GL_FLOAT;
		case Karma::ShaderDataType::Float3:
			return GL_FLOAT;
		case Karma::ShaderDataType::Float4:
			return GL_FLOAT;
		case Karma::ShaderDataType::Mat3:
			return GL_FLOAT;
		case Karma::ShaderDataType::Mat4:
			return GL_FLOAT;
		case Karma::ShaderDataType::Int:
			return GL_INT;
		case Karma::ShaderDataType::Int2:
			return GL_INT;
		case Karma::ShaderDataType::Int3:
			return GL_INT;
		case Karma::ShaderDataType::Int4:
			return GL_INT;
		case Karma::ShaderDataType::Bool:
			return GL_INT;
		}

		KR_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &m_RendererID);
	}
	
	void OpenGLVertexArray::Bind() const
	{
		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::UnBind() const
	{
		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
	{
		KR_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "VertexBufferLayout empty.");

		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();

		uint32_t index = 0;
		auto layout = vertexBuffer->GetLayout();
		for (const auto& element : layout)
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(index,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)element.Offset);
			index++;
		}
		int a = 10;
		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
	{
		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}
}