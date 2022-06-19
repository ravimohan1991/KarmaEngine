#pragma once

#include "Karma/Renderer/Buffer.h"
#include "Karma/Core.h"
#include "Karma/Renderer/Shader.h"
#include "Karma/Renderer/Mesh.h"

#include <memory>

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

		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) = 0; // For legacy purposes. Use Mesh abstraction.
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) = 0; // For legacy purposes. Use Mesh abstraction.

		virtual void SetMesh(std::shared_ptr<Mesh> mesh) = 0;

		virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const = 0;
		virtual const IndexBuffer* GetIndexBuffer() const = 0;

		virtual void SetShader(std::shared_ptr<Shader> shader) = 0;

		static VertexArray* Create();
	};
}