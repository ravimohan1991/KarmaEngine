#include "Scene.h"

namespace Karma
{
	Scene::Scene()
	{
		m_WindowResize = false;
	}

	Scene::~Scene()
	{
	}

	std::shared_ptr<VertexArray> Scene::GetRenderableVertexArray() const
	{
		// Get the first for now
		return m_VertexArrays.at(0);
	}

	std::shared_ptr<Camera> Scene::GetSceneCamera() const
	{
		// Get the first for now
		return m_Cameras.at(0);
	}

	void Scene::AddVertexArray(std::shared_ptr<VertexArray> vertexArray)
	{
		m_VertexArrays.push_back(vertexArray);
	}

	void Scene::AddCamera(std::shared_ptr<Camera> camera)
	{
		m_Cameras.push_back(camera);
	}
}
