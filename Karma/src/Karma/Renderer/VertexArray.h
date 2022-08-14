#pragma once

#include "krpch.h"

#include "Karma/Renderer/Buffer.h"
#include "Karma/Renderer/Shader.h"
#include "Karma/Renderer/Mesh.h"
#include "Karma/Renderer/Material.h"

namespace Karma
{
	class KARMA_API VertexArray
	{
	public:
		virtual ~VertexArray()
		{
		}

		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;

		// For legacy purposes. Use Mesh abstraction.
		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) = 0;
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) = 0;
		virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const = 0;
		virtual const IndexBuffer* GetIndexBuffer() const = 0;

		virtual void SetMesh(std::shared_ptr<Mesh> mesh) = 0;

		// For legacy purposes. Use Material abstraction.
		virtual void SetShader(std::shared_ptr<Shader> shader) = 0;

		virtual void SetMaterial(std::shared_ptr<Material> material) = 0;

		virtual void UpdateProcessAndSetReadyForSubmission() const = 0;

		// Getters
		//virtual std::shared_ptr<Mesh> GetMesh() const = 0;
		virtual std::shared_ptr<Material> GetMaterial() const = 0;

		static VertexArray* Create();
	};
}