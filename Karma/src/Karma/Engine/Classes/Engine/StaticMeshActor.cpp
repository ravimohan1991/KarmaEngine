#include "StaticMeshActor.h"
#include "Mesh.h"

#include "StaticMeshComponent.h"

namespace Karma
{
	AStaticMeshActor::AStaticMeshActor(const std::string& name)
		: AActor()
	{
	}

	AStaticMeshActor::~AStaticMeshActor()
	{
		// Cleanup code if necessary
	}

	void AStaticMeshActor::LoadMeshFromFile(const std::string& filePath)
	{
		std::shared_ptr<Mesh> loadedMesh;
		loadedMesh.reset(new Mesh(filePath));

		if (!m_StaticMeshComponent)
		{
			// need to write code to destroy UObjects later
			m_StaticMeshComponent = NewObject<UStaticMeshComponent>(nullptr, UStaticMeshComponent::StaticClass(), "StaticMeshComponent");
		}

		m_StaticMeshComponent->SetStaticMesh(loadedMesh);
	}
}