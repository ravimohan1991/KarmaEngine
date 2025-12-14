#include "StaticMeshActor.h"
#include "Mesh.h"

namespace Karma
{
	AStaticMeshActor::AStaticMeshActor(const std::string& name)
		: AActor()
	{
	}

	void AStaticMeshActor::LoadMeshFromFile(const std::string& filePath)
	{
		std::shared_ptr<Mesh> loadedMesh;
		loadedMesh.reset(new Mesh(filePath));

		// We create uniform buffer object for model's transformation matrix
		m_MeshTransformUniform.reset(UniformBufferObject::Create({ ShaderDataType::Mat4 }, 2));// Binding point 2 and gets registered in VulkanContext during its initialization

		m_ActorTranformUniform = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 1.f));
		m_ActorTranformUniform = glm::scale(m_ActorTranformUniform, glm::vec3(0.05f));
		
		UBODataPointer uModelMatrix(&m_ActorTranformUniform);
		m_MeshTransformUniform->UpdateUniforms(uModelMatrix);// set the model matrix (Actor Transform) uniform

		if (!m_StaticMeshComponent)
		{
			// need to write code to destroy UObjects later
			m_StaticMeshComponent = NewObject<UStaticMeshComponent>(nullptr, UStaticMeshComponent::StaticClass(), "StaticMeshComponent");
		}

		m_StaticMeshComponent->SetStaticMesh(loadedMesh);
	}
}
