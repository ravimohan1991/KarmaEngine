#include "Renderer.h"

namespace Karma
{
	Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData();
	std::shared_ptr<Scene> Renderer::m_Scene = nullptr;

	void Renderer::BeginScene(std::shared_ptr<Scene> scene)
	{
		//m_SceneData->ProjectionMatrix = camera.GetProjectionMatrix();
		RenderCommand::BeginScene();
	}

	void Renderer::EndScene()
	{
		RenderCommand::EndScene();
	}

	void Renderer::Submit(std::shared_ptr<Scene> scene)
	{
		RenderCommand::DrawIndexed(scene->GetRenderableVertexArray());
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