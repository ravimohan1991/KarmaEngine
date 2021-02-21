#pragma once

#include "Karma/Renderer/RendererAPI.h"
#include "Karma/Core.h"

namespace Karma
{
	class KARMA_API VulkanRendererAPI : public RendererAPI
	{
	public:
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) override;

	private:
		size_t m_CurrentFrame = 0;
	};
}