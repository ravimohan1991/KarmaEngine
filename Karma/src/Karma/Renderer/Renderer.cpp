#include "Renderer.h"

namespace Karma
{
	Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData();

	void Renderer::BeginScene(Camera& camera)
	{
		m_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
		RenderCommand::BeginScene();
	}

	void Renderer::EndScene()
	{
		RenderCommand::EndScene();
	}

	void Renderer::Submit(const std::shared_ptr<VertexArray>& vertexArray, const std::shared_ptr<Shader>& shader, glm::mat4& transform)
	{
		shader->Bind();
		shader->GetUniformBufferObject()->UploadUniformBuffer();
		
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

	void Renderer::DeleteData()
	{
		if (m_SceneData)
		{
			delete m_SceneData;
			m_SceneData = 0;
		}
	}
}