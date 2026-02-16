#include "StaticMeshActor.h"
#include "Mesh.h"

#include <glm/gtc/random.hpp>

namespace Karma
{
	AStaticMeshActor::AStaticMeshActor(const std::string& name)
		: AActor()
	{
		if (!m_StaticMeshComponent)
		{
			// Unreal uses CreateDefaultSubobject for creating components in Actors
			// may need to implement similar functionality later
			m_StaticMeshComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), "StaticMeshComponent");
			m_StaticMeshComponent->SetOwner(this);	
		}

		m_RootComponent = m_StaticMeshComponent;
	}

	void AStaticMeshActor::Tick(float DeltaSeconds)
	{
		FTransform AnimTransform = GetTransform();
		
		AnimTransform.SetRotation(TRotator(glm::vec3(0.f, 0.f, m_RotationAngle)));
		
		//SetActorTransform(AnimTransform);

		m_RotationAngle += 0.78f * DeltaSeconds * m_RotationSpeed;
		
		if(m_RotationAngle > 360.f)
		{
			m_RotationAngle -= 360.f;
		}
	}

	void AStaticMeshActor::LoadMeshFromFile(const std::string& filePath)
	{
		std::shared_ptr<Mesh> loadedMesh;
		loadedMesh.reset(new Mesh(filePath));

		m_StaticMeshComponent->SetStaticMesh(loadedMesh);
		m_RotationSpeed = glm::linearRand(0.2f, 1.0f);
	}
}
