#include "Scene.h"
#include "StaticMeshActor.h"

namespace Karma
{
	Scene::Scene()
	{
		m_WindowResize = false;
	}

	Scene::~Scene()
	{
	}

	std::shared_ptr<Camera> Scene::GetSceneCamera() const
	{
		// Get the first for now
		return m_Cameras.at(0);
	}

	void Scene::AddStaticMeshActor(AStaticMeshActor* smActor)
	{
		m_SMActors.push_back(smActor);
	}

	void Scene::AddCamera(std::shared_ptr<Camera> camera)
	{
		m_Cameras.push_back(camera);
	}
}
