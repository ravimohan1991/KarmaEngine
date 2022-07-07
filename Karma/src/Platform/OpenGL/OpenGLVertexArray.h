#pragma once

#include "Karma/Renderer/VertexArray.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Karma
{
	class KARMA_API OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		virtual ~OpenGLVertexArray();

		virtual void Bind() const override;
		virtual void UnBind() const override;

		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;

		virtual void SetMesh(std::shared_ptr<Mesh> mesh) override;
		virtual void SetMaterial(std::shared_ptr<Material> material) override;

		virtual void SetShader(std::shared_ptr<Shader> shader) override {}

		virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
		virtual const IndexBuffer* GetIndexBuffer() const override { return m_IndexBuffer.get(); }

		virtual std::shared_ptr<Material> GetMaterial() const override { return m_Materials.at(0); }

		virtual void UpdateProcessAndSetReadyForSubmission() const override;

	private:
		uint32_t m_RendererID;

		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;

		// Material relevant members
		std::vector<std::shared_ptr<Material>> m_Materials;
		std::vector<std::shared_ptr<OpenGLShader>> m_Shaders;
		std::shared_ptr<OpenGLShader> m_Shader;

	};
}