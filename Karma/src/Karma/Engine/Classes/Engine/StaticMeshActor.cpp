#include "StaticMeshActor.h"
#include "Mesh.h"

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
		//m_ActorTranformUniform = glm::rotate(m_ActorTranformUniform, glm::radians(7.8f * DeltaSeconds), glm::vec3(0.f, 0.f, 1.f));
		
		FTransform AnimTransform = FTransform::Identity();
		
		AnimTransform.SetRotation(TRotator(glm::vec3(0.f, 0.f, m_RotationAngle)));
		
		SetActorTransform(AnimTransform);

		m_ActorTranformUniform = GetTransform().ToMatrixWithScale();
		
		m_RotationAngle += 0.78f * DeltaSeconds;
		
		if(m_RotationAngle > 360.f)
		{
			m_RotationAngle -= 360.f;
		}
	}

	void AStaticMeshActor::LoadMeshFromFile(const std::string& filePath)
	{
		std::shared_ptr<Mesh> loadedMesh;
		loadedMesh.reset(new Mesh(filePath));

		// We create uniform buffer object for model's transformation matrix
		m_MeshTransformUniform.reset(UniformBufferObject::Create({ ShaderDataType::Mat4 }, 0));// Binding point 2 and gets registered in VulkanContext during its initialization

		//m_ActorTranformUniform = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 1.f));
		//m_ActorTranformUniform = glm::scale(m_ActorTranformUniform, glm::vec3(0.25f));
		
		m_ActorTranformUniform = GetTransform().ToMatrixWithScale();
		
		UBODataPointer uModelMatrix(&m_ActorTranformUniform);
		m_MeshTransformUniform->UpdateUniforms(uModelMatrix);// set the model matrix (Actor Transform) uniform

		m_StaticMeshComponent->SetStaticMesh(loadedMesh);
	}
}
