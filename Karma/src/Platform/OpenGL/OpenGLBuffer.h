#pragma once

#include "Karma/Renderer/Buffer.h"

namespace Karma
{
	class KARMA_API OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(float* vertices, uint32_t size);
		virtual ~OpenGLVertexBuffer();

		virtual void Bind() const override;
		virtual void UnBind() const override;

		virtual const BufferLayout& GetLayout() const override 
		{
			return m_Layout;
		}
		virtual void SetLayout(const BufferLayout& layout) override
		{
			m_Layout = layout;
		}

	private:
		uint32_t m_RendererID;
		BufferLayout m_Layout;
	};

	class KARMA_API OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
		virtual ~OpenGLIndexBuffer();

		virtual void Bind() const override;
		virtual void UnBind() const override;

		virtual uint32_t GetCount() const override { return m_Count; }

	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};

	struct KARMA_API OpenGLUniformBuffer : public UniformBufferObject
	{
	public:
		OpenGLUniformBuffer(std::vector<ShaderDataType> dataTypes, uint32_t bindingPointIndex);
		uint32_t GetUniformsID() const { return m_UniformsID; }
		virtual ~OpenGLUniformBuffer();

	private:
		void GenerateUniformBufferObject();
		void BindUniformBufferObject() const;

	private:
		uint32_t m_UniformsID;
	};
}