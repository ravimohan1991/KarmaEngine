#include "Renderer.h"

namespace Karma
{
	Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData();

	void Renderer::BeginScene(std::shared_ptr<Scene> scene)
	{
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
