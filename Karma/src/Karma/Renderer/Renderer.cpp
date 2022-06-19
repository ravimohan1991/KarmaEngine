#include "Renderer.h"

namespace Karma
{
	Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData();

	void Renderer::BeginScene(Camera& camera)
	{
		m_SceneData->ProjectionMatrix = camera.GetProjectionMatrix();
		RenderCommand::BeginScene();
	}

	void Renderer::EndScene()
	{
		RenderCommand::EndScene();
	}

	void Renderer::Submit(const std::shared_ptr<VertexArray>& vertexArray)
	{	
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