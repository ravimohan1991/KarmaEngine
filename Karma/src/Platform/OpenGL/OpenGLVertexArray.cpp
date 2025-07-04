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
		case Karma::ShaderDataType::None:
			return GL_NONE;
		}

		KR_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		//glCreateVertexArrays(1, &m_RendererID);
		glGenVertexArrays(1, &m_RendererID);
		Bind();
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
		const auto& layout = vertexBuffer->GetLayout();
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
		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
	{
		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}

	void OpenGLVertexArray::SetMesh(std::shared_ptr<Mesh> mesh)
	{
		// Vertexbuffer stuff
		KR_CORE_ASSERT(mesh->GetVertexBuffer()->GetLayout().GetElements().size(), "VertexBufferLayout empty.");

		glBindVertexArray(m_RendererID);
		mesh->GetVertexBuffer()->Bind();

		uint32_t index = 0;
		const auto& layout = mesh->GetVertexBuffer()->GetLayout();
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
		// We are seperating VertexBuffers from Mesh.  Hopefully useful for batch rendering!
		m_VertexBuffers.push_back(mesh->GetVertexBuffer());


		// Index buffer stuff
		glBindVertexArray(m_RendererID);
		mesh->GetIndexBuffer()->Bind();

		// May need modificaitons for batch rendering later.
		m_IndexBuffer = mesh->GetIndexBuffer();
	}

	void OpenGLVertexArray::SetMaterial(std::shared_ptr<Material> material)
	{
		m_Materials.push_back(material);
		m_Shader = std::static_pointer_cast<OpenGLShader>(material->GetShader(0));
	}

	void OpenGLVertexArray::UpdateProcessAndSetReadyForSubmission() const
	{
		// May need entry point for Object's world transform
		// also may need to shift a level up
		m_Materials.at(0)->OnUpdate();
		m_Materials.at(0)->ProcessForSubmission();
	}
}
