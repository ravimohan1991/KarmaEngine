#include "StaticMeshActor.h"
#include "Mesh.h"

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

		// We create uniform buffer object for model's transformation matrix
		std::shared_ptr<UniformBufferObject> meshTransformUniform;
		meshTransformUniform.reset(UniformBufferObject::Create({ ShaderDataType::Mat4 }, 2));// Binding point 2 and gets registered in VulkanContext during its initialization

		UBODataPointer uModelMatrix(&GetTransform());
		meshTransformUniform->UpdateUniforms(uModelMatrix);// set the model matrix (Actor Transform) uniform

		//if (!m_StaticMeshComponent)
		//{
			// need to write code to destroy UObjects later
			m_StaticMeshComponent = NewObject<UStaticMeshComponent>(nullptr, UStaticMeshComponent::StaticClass(), "StaticMeshComponent");
		//}

		m_StaticMeshComponent->SetStaticMesh(loadedMesh);
	}
}