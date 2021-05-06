#include "Renderer.h"

namespace Karma
{
	Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData();

	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		m_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::Submit(const std::shared_ptr<VertexArray>& vertexArray, const std::shared_ptr<Shader>& shader, glm::mat4& transform)
	{
		if (shader)
		{
			shader->Bind();
			UBODataPointer viewProjection(&m_SceneData->ViewProjectionMatrix);
			UBODataPointer trans(&transform);
			shader->GetUniformBuffer()->UpdateUniforms(viewProjection, trans);
			shader->UploadUniformBuffer();
		}
		
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}
}